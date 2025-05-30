/*	$NetBSD: glob.c,v 1.38.10.1 2019/06/10 22:05:19 christos Exp $	*/

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _NETBSD_SOURCE
#define WIN32_UNISTD_MAP

#include <sys/cdefs.h>

/*
 * glob(3) -- a superset of the one defined in POSIX 1003.2.
 *
 * The [!...] convention to negate a range is supported (SysV, Posix, ksh).
 *
 * Optional extra services, controlled by flags not defined by POSIX:
 *
 * GLOB_MAGCHAR:
 *	Set in gl_flags if pattern contained a globbing character.
 * GLOB_NOMAGIC:
 *	Same as GLOB_NOCHECK, but it will only append pattern if it did
 *	not contain any magic characters.  [Used in csh style globbing]
 * GLOB_ALTDIRFUNC:
 *	Use alternately specified directory access functions.
 * GLOB_TILDE:
 *	expand ~user/foo to the /home/dir/of/user/foo
 * GLOB_BRACE:
 *	expand {1,2}{a,b} to 1a 1b 2a 2b 
 * GLOB_PERIOD:
 *	allow metacharacters to match leading dots in filenames.
 * GLOB_NO_DOTDIRS:
 *	. and .. are hidden from wildcards, even if GLOB_PERIOD is set.
 * gl_matchc:
 *	Number of matches in the current invocation of glob.
 */

    //#include "namespace.h"
#include <sys/param.h>
#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
    //#include <glob.h>
#include "glob.h"
#include <pwd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_NBTOOL_CONFIG_H
#define NO_GETPW_R
#endif

#define	GLOB_LIMIT_STRING	524288	/* number of readdirs */
#define	GLOB_LIMIT_STAT		128	/* number of stat system calls */
#define	GLOB_LIMIT_READDIR	65536	/* total buffer size of path strings */
#define	GLOB_LIMIT_PATH		1024	/* number of path elements */
#define GLOB_LIMIT_BRACE	128	/* Number of brace calls */

struct glob_limit {
	size_t l_string;
	size_t l_stat;	
	size_t l_readdir;	
	size_t l_brace;
};

/*
 * XXX: For NetBSD 1.4.x compatibility. (kill me l8r)
 */
#ifndef _DIAGASSERT
#define _DIAGASSERT(a)
#endif

#define	DOLLAR		'$'
#define	DOT		'.'
#define	EOS		'\0'
#define	LBRACKET	'['
#define	NOT		'!'
#define	QUESTION	'?'
#define	QUOTE		'\\'
#define	RANGE		'-'
#define	RBRACKET	']'
#define	SEP		'/'
#define	STAR		'*'
#define	TILDE		'~'
#define	UNDERSCORE	'_'
#define	LBRACE		'{'
#define	RBRACE		'}'
#define	SLASH		'/'
#define	COMMA		','

#ifndef USE_8BIT_CHARS

#define	M_QUOTE		0x8000
#define	M_PROTECT	0x4000
#define	M_MASK		0xffff
#define	M_ASCII		0x00ff

typedef unsigned short Char;

#else

#define	M_QUOTE		(Char)0x80
#define	M_PROTECT	(Char)0x40
#define	M_MASK		(Char)0xff
#define	M_ASCII		(Char)0x7f

typedef char Char;

#endif


#define	CHAR(c)		((Char)((c)&M_ASCII))
#define	META(c)		((Char)((c)|M_QUOTE))
#define	M_ALL		META('*')
#define	M_END		META(']')
#define	M_NOT		META('!')
#define	M_ONE		META('?')
#define	M_RNG		META('-')
#define	M_SET		META('[')
#define	ismeta(c)	(((c)&M_QUOTE) != 0)


static int	 compare(const void *, const void *);
static int	 g_Ctoc(const Char *, char *, size_t);
static int	 g_lstat(Char *, __gl_stat_t  *, glob_t *);
static DIR	*g_opendir(Char *, glob_t *);
static Char	*g_strchr(const Char *, int);
static int	 g_stat(Char *, __gl_stat_t *, glob_t *);
static int	 glob0(const Char *, glob_t *, struct glob_limit *);
static int	 glob1(Char *, glob_t *, struct glob_limit *);
static int	 glob2(Char *, Char *, Char *, const Char *, glob_t *,
    struct glob_limit *);
static int	 glob3(Char *, Char *, Char *, const Char *, const Char *,
    const Char *, glob_t *, struct glob_limit *);
static int	 globextend(const Char *, glob_t *, struct glob_limit *);
static int       globtilde(const Char **, const Char *, Char *, size_t,
    glob_t *);
static int	 globexp1(const Char *, glob_t *, struct glob_limit *);
static int	 globexp2(const Char *, const Char *, glob_t *, int *,
    struct glob_limit *);
static int	 match(const Char *, const Char *, const Char *);
#ifdef DEBUG
static void	 qprintf(const char *, Char *);
#endif

int
glob(const char * __restrict pattern, int flags, int (*errfunc)(const char *,
    int), glob_t * __restrict pglob)
{
	const unsigned char *patnext;
	int c;
	Char *bufnext, *bufend, patbuf[MAXPATHLEN+1];
	struct glob_limit limit = { 0, 0, 0, 0 };

	_DIAGASSERT(pattern != NULL);

	patnext = (const unsigned char *) pattern;
	if (!(flags & GLOB_APPEND)) {
		pglob->gl_pathc = 0;
		pglob->gl_pathv = NULL;
		if (!(flags & GLOB_DOOFFS))
			pglob->gl_offs = 0;
	}
	pglob->gl_flags = flags & ~GLOB_MAGCHAR;
	pglob->gl_errfunc = errfunc;
	pglob->gl_matchc = 0;

	bufnext = patbuf;
	bufend = bufnext + MAXPATHLEN;
	if (flags & GLOB_NOESCAPE) {
		while (bufnext < bufend && (c = *patnext++) != EOS) 
			*bufnext++ = c;
	} else {
		/* Protect the quoted characters. */
		while (bufnext < bufend && (c = *patnext++) != EOS) 
			if (c == QUOTE) {
				if ((c = *patnext++) == EOS) {
					c = QUOTE;
					--patnext;
				}
				*bufnext++ = c | M_PROTECT;
			}
			else
				*bufnext++ = c;
	}
	*bufnext = EOS;

	if (flags & GLOB_BRACE)
	    return globexp1(patbuf, pglob, &limit);
	else
	    return glob0(patbuf, pglob, &limit);
}

/*
 * Expand recursively a glob {} pattern. When there is no more expansion
 * invoke the standard globbing routine to glob the rest of the magic
 * characters
 */
static int
globexp1(const Char *pattern, glob_t *pglob, struct glob_limit *limit)
{
	const Char* ptr = pattern;
	int rv;

	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(pglob != NULL);

	if ((pglob->gl_flags & GLOB_LIMIT) &&
	    limit->l_brace++ >= GLOB_LIMIT_BRACE) {
		errno = 0;
		return GLOB_NOSPACE;
	}

	/* Protect a single {}, for find(1), like csh */
	if (pattern[0] == LBRACE && pattern[1] == RBRACE && pattern[2] == EOS)
		return glob0(pattern, pglob, limit);

	while ((ptr = (const Char *) g_strchr(ptr, LBRACE)) != NULL)
		if (!globexp2(ptr, pattern, pglob, &rv, limit))
			return rv;

	return glob0(pattern, pglob, limit);
}


/*
 * Recursive brace globbing helper. Tries to expand a single brace.
 * If it succeeds then it invokes globexp1 with the new pattern.
 * If it fails then it tries to glob the rest of the pattern and returns.
 */
static int
globexp2(const Char *ptr, const Char *pattern, glob_t *pglob, int *rv,
    struct glob_limit *limit)
{
	int     i;
	Char   *lm, *ls;
	const Char *pe, *pm, *pl;
	Char    patbuf[MAXPATHLEN + 1];

	_DIAGASSERT(ptr != NULL);
	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(pglob != NULL);
	_DIAGASSERT(rv != NULL);

	/* copy part up to the brace */
	for (lm = patbuf, pm = pattern; pm != ptr; *lm++ = *pm++)
		continue;
	ls = lm;

	/* Find the balanced brace */
	for (i = 0, pe = ++ptr; *pe; pe++)
		if (*pe == LBRACKET) {
			/* Ignore everything between [] */
			for (pm = pe++; *pe != RBRACKET && *pe != EOS; pe++)
				continue;
			if (*pe == EOS) {
				/* 
				 * We could not find a matching RBRACKET.
				 * Ignore and just look for RBRACE
				 */
				pe = pm;
			}
		}
		else if (*pe == LBRACE)
			i++;
		else if (*pe == RBRACE) {
			if (i == 0)
				break;
			i--;
		}

	/* Non matching braces; just glob the pattern */
	if (i != 0 || *pe == EOS) {
		/*
		 * we use `pattern', not `patbuf' here so that that
		 * unbalanced braces are passed to the match
		 */
		*rv = glob0(pattern, pglob, limit);
		return 0;
	}

	for (i = 0, pl = pm = ptr; pm <= pe; pm++) {
		switch (*pm) {
		case LBRACKET:
			/* Ignore everything between [] */
			for (pl = pm++; *pm != RBRACKET && *pm != EOS; pm++)
				continue;
			if (*pm == EOS) {
				/* 
				 * We could not find a matching RBRACKET.
				 * Ignore and just look for RBRACE
				 */
				pm = pl;
			}
			break;

		case LBRACE:
			i++;
			break;

		case RBRACE:
			if (i) {
				i--;
				break;
			}
			/* FALLTHROUGH */
		case COMMA:
			if (i && *pm == COMMA)
				break;
			else {
				/* Append the current string */
				for (lm = ls; (pl < pm); *lm++ = *pl++)
					continue;
				/* 
				 * Append the rest of the pattern after the
				 * closing brace
				 */
				for (pl = pe + 1; (*lm++ = *pl++) != EOS;)
					continue;

				/* Expand the current pattern */
#ifdef DEBUG
				qprintf("globexp2", patbuf);
#endif
				*rv = globexp1(patbuf, pglob, limit);

				/* move after the comma, to the next string */
				pl = pm + 1;
			}
			break;

		default:
			break;
		}
	}
	*rv = 0;
	return 0;
}



/*
 * expand tilde from the passwd file.
 */
static int
globtilde(const Char **qpatnext, const Char *pattern, Char *patbuf,
    size_t patsize, glob_t *pglob)
{
	struct passwd *pwd;
	const char *h;
	const Char *p;
	Char *b;
	char *d;
	Char *pend = &patbuf[patsize / sizeof(Char)];
#ifndef NO_GETPW_R
	struct passwd pwres;
	char pwbuf[1024];
#endif

	pend--;

	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(patbuf != NULL);
	_DIAGASSERT(pglob != NULL);
	*qpatnext = pattern;

	if (*pattern != TILDE || !(pglob->gl_flags & GLOB_TILDE))
		return 0;

	/* Copy up to the end of the string or / */
	for (p = pattern + 1, d = (char *)(void *)patbuf; 
	     d < (char *)(void *)pend && *p && *p != SLASH; *d++ = (char)(*p++))
		continue;

	if (d == (char *)(void *)pend)
		return GLOB_ABEND;

	*d = EOS;
	d = (char *)(void *)patbuf;

	if (*d == EOS) {
		/* 
		 * handle a plain ~ or ~/ by expanding $HOME 
		 * first and then trying the password file
		 */
		if ((h = getenv("HOME")) == NULL) {
#ifdef NO_GETPW_R
			if ((pwd = getpwuid(getuid())) == NULL)
#else
			if (getpwuid_r(getuid(), &pwres, pwbuf, sizeof(pwbuf),
			    &pwd) != 0 || pwd == NULL)
#endif
				goto nouser;
			h = pwd->pw_dir;
		}
	}
	else {
		/*
		 * Expand a ~user
		 */
#ifdef NO_GETPW_R
		if ((pwd = getpwnam(d)) == NULL)
#else
		if (getpwnam_r(d, &pwres, pwbuf, sizeof(pwbuf), &pwd) != 0 ||
		    pwd == NULL)
#endif
			goto nouser;
		h = pwd->pw_dir;
	}

	/* Copy the home directory */
	for (b = patbuf; b < pend && *h; *b++ = *h++)
		continue;

	if (b == pend)
		return GLOB_ABEND;
	
	/* Append the rest of the pattern */
	while (b < pend && (*b++ = *p++) != EOS)
		continue;

	if (b == pend)
		return GLOB_ABEND;

	*qpatnext = patbuf;
	return 0;
nouser:
	return (pglob->gl_flags & GLOB_TILDE_CHECK) ?  GLOB_NOMATCH : 0;
}
	

/*
 * The main glob() routine: compiles the pattern (optionally processing
 * quotes), calls glob1() to do the real pattern matching, and finally
 * sorts the list (unless unsorted operation is requested).  Returns 0
 * if things went well, nonzero if errors occurred.  It is not an error
 * to find no matches.
 */
static int
glob0(const Char *pattern, glob_t *pglob, struct glob_limit *limit)
{
	const Char *qpatnext;
	int c, error;
	__gl_size_t oldpathc;
	Char *bufnext, patbuf[MAXPATHLEN+1];

	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(pglob != NULL);

	if ((error = globtilde(&qpatnext, pattern, patbuf, sizeof(patbuf),
	    pglob)) != 0)
		return error;
	oldpathc = pglob->gl_pathc;
	bufnext = patbuf;

	/* We don't need to check for buffer overflow any more. */
	while ((c = *qpatnext++) != EOS) {
		switch (c) {
		case LBRACKET:
			c = *qpatnext;
			if (c == NOT)
				++qpatnext;
			if (*qpatnext == EOS ||
			    g_strchr(qpatnext+1, RBRACKET) == NULL) {
				*bufnext++ = LBRACKET;
				if (c == NOT)
					--qpatnext;
				break;
			}
			*bufnext++ = M_SET;
			if (c == NOT)
				*bufnext++ = M_NOT;
			c = *qpatnext++;
			do {
				*bufnext++ = CHAR(c);
				if (*qpatnext == RANGE &&
				    (c = qpatnext[1]) != RBRACKET) {
					*bufnext++ = M_RNG;
					*bufnext++ = CHAR(c);
					qpatnext += 2;
				}
			} while ((c = *qpatnext++) != RBRACKET);
			pglob->gl_flags |= GLOB_MAGCHAR;
			*bufnext++ = M_END;
			break;
		case QUESTION:
			pglob->gl_flags |= GLOB_MAGCHAR;
			*bufnext++ = M_ONE;
			break;
		case STAR:
			pglob->gl_flags |= GLOB_MAGCHAR;
			/* collapse adjacent stars to one [or three if globstar]
			 * to avoid exponential behavior
			 */
			if (bufnext == patbuf || bufnext[-1] != M_ALL ||
			    ((pglob->gl_flags & GLOB_STAR) != 0 && 
			    (bufnext - 1 == patbuf || bufnext[-2] != M_ALL ||
			    bufnext - 2 == patbuf || bufnext[-3] != M_ALL)))
				*bufnext++ = M_ALL;
			break;
		default:
			*bufnext++ = CHAR(c);
			break;
		}
	}
	*bufnext = EOS;
#ifdef DEBUG
	qprintf("glob0", patbuf);
#endif

	if ((error = glob1(patbuf, pglob, limit)) != 0)
		return error;

	if (pglob->gl_pathc == oldpathc) {	
		/*
		 * If there was no match we are going to append the pattern 
		 * if GLOB_NOCHECK was specified or if GLOB_NOMAGIC was
		 * specified and the pattern did not contain any magic
		 * characters GLOB_NOMAGIC is there just for compatibility
		 * with csh.
		 */
		if ((pglob->gl_flags & GLOB_NOCHECK) ||
		    ((pglob->gl_flags & (GLOB_NOMAGIC|GLOB_MAGCHAR))
		     == GLOB_NOMAGIC)) {
			return globextend(pattern, pglob, limit);
		} else {
			return GLOB_NOMATCH;
		}
	} else if (!(pglob->gl_flags & GLOB_NOSORT)) {
		qsort(pglob->gl_pathv + pglob->gl_offs + oldpathc,
		    (size_t)pglob->gl_pathc - oldpathc, sizeof(char *),
		    compare);
	}

	return 0;
}

static int
compare(const void *p, const void *q)
{

	_DIAGASSERT(p != NULL);
	_DIAGASSERT(q != NULL);

	return strcoll(*(const char * const *)p, *(const char * const *)q);
}

static int
glob1(Char *pattern, glob_t *pglob, struct glob_limit *limit)
{
	Char pathbuf[MAXPATHLEN+1];

	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(pglob != NULL);

	/* A null pathname is invalid -- POSIX 1003.1 sect. 2.4. */
	if (*pattern == EOS)
		return 0;
	/*
	 * we save one character so that we can use ptr >= limit,
	 * in the general case when we are appending non nul chars only.
	 */
	return glob2(pathbuf, pathbuf,
	    pathbuf + (sizeof(pathbuf) / sizeof(*pathbuf)) - 1, pattern,
	    pglob, limit);
}

/*
 * The functions glob2 and glob3 are mutually recursive; there is one level
 * of recursion for each segment in the pattern that contains one or more
 * meta characters.
 */
static int
glob2(Char *pathbuf, Char *pathend, Char *pathlim, const Char *pattern,
    glob_t *pglob, struct glob_limit *limit)
{
	__gl_stat_t sb;
	const Char *p;
	Char *q;
	int anymeta;

	_DIAGASSERT(pathbuf != NULL);
	_DIAGASSERT(pathend != NULL);
	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(pglob != NULL);

#ifdef DEBUG
	qprintf("glob2", pathbuf);
#endif
	/*
	 * Loop over pattern segments until end of pattern or until
	 * segment with meta character found.
	 */
	for (anymeta = 0;;) {
		if (*pattern == EOS) {		/* End of pattern? */
			*pathend = EOS;
			if (g_lstat(pathbuf, &sb, pglob))
				return 0;
		
			if ((pglob->gl_flags & GLOB_LIMIT) &&
			    limit->l_stat++ >= GLOB_LIMIT_STAT) {
				errno = 0;
				*pathend++ = SEP;
				*pathend = EOS;
				return GLOB_NOSPACE;
			}
			if (((pglob->gl_flags & GLOB_MARK) &&
			    pathend[-1] != SEP) && (S_ISDIR(sb.st_mode) ||
			    (S_ISLNK(sb.st_mode) &&
			    (g_stat(pathbuf, &sb, pglob) == 0) &&
			    S_ISDIR(sb.st_mode)))) {
				if (pathend >= pathlim)
					return GLOB_ABORTED;
				*pathend++ = SEP;
				*pathend = EOS;
			}
			++pglob->gl_matchc;
			return globextend(pathbuf, pglob, limit);
		}

		/* Find end of next segment, copy tentatively to pathend. */
		q = pathend;
		p = pattern;
		while (*p != EOS && *p != SEP) {
			if (ismeta(*p))
				anymeta = 1;
			if (q >= pathlim)
				return GLOB_ABORTED;
			*q++ = *p++;
		}

                if (!anymeta) {
			pathend = q;
			pattern = p;
			while (*pattern == SEP) {
				if (pathend >= pathlim)
					return GLOB_ABORTED;
				*pathend++ = *pattern++;
			}
		} else			/* Need expansion, recurse. */
			return glob3(pathbuf, pathend, pathlim, pattern, p,
			    pattern, pglob, limit);
	}
	/* NOTREACHED */
}

static int
glob3(Char *pathbuf, Char *pathend, Char *pathlim, const Char *pattern,
    const Char *restpattern, const Char *pglobstar, glob_t *pglob,
    struct glob_limit *limit)
{
	struct dirent *dp;
	DIR *dirp;
	__gl_stat_t sbuf;
	int error;
	char buf[MAXPATHLEN];
	int globstar = 0;
	int chase_symlinks = 0;
	const Char *termstar = NULL;

	/*
	 * The readdirfunc declaration can't be prototyped, because it is
	 * assigned, below, to two functions which are prototyped in glob.h
	 * and dirent.h as taking pointers to differently typed opaque
	 * structures.
	 */
	struct dirent *(*readdirfunc)(void *);

	_DIAGASSERT(pathbuf != NULL);
	_DIAGASSERT(pathend != NULL);
	_DIAGASSERT(pattern != NULL);
	_DIAGASSERT(restpattern != NULL);
	_DIAGASSERT(pglob != NULL);

	*pathend = EOS;
	errno = 0;
	    
	while (pglobstar < restpattern) {
		if ((pglobstar[0] & M_MASK) == M_ALL &&
		    (pglobstar[1] & M_MASK) == M_ALL) {
			globstar = 1;
			chase_symlinks = (pglobstar[2] & M_MASK) == M_ALL;
			termstar = pglobstar + (2 + chase_symlinks);
			break;
		}
		pglobstar++;
	} 

	if (globstar) {
		error = pglobstar == pattern && termstar == restpattern ?
		    *restpattern == EOS ?
		    glob2(pathbuf, pathend, pathlim, restpattern - 1, pglob,
		    limit) :
		    glob2(pathbuf, pathend, pathlim, restpattern + 1, pglob,
		    limit) :
		    glob3(pathbuf, pathend, pathlim, pattern, restpattern,
		    termstar, pglob, limit);
		if (error)
			return error;
		*pathend = EOS;
	}

	if (*pathbuf && (g_lstat(pathbuf, &sbuf, pglob) ||
	    !S_ISDIR(sbuf.st_mode)
#ifdef S_IFLINK
	     && ((globstar && !chase_symlinks) || !S_ISLNK(sbuf.st_mode))
#endif
	    ))
		return 0;

	if ((dirp = g_opendir(pathbuf, pglob)) == NULL) {
		if (pglob->gl_errfunc) {
			if (g_Ctoc(pathbuf, buf, sizeof(buf)))
				return GLOB_ABORTED;
			if (pglob->gl_errfunc(buf, errno) ||
			    pglob->gl_flags & GLOB_ERR)
				return GLOB_ABORTED;
		}
		/*
		 * Posix/XOpen: glob should return when it encounters a
		 * directory that it cannot open or read
		 * XXX: Should we ignore ENOTDIR and ENOENT though?
		 * I think that Posix had in mind EPERM...
		 */
		if (pglob->gl_flags & GLOB_ERR)
			return GLOB_ABORTED;

		return 0;
	}

	error = 0;

	/* Search directory for matching names. */
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		readdirfunc = pglob->gl_readdir;
	else
		readdirfunc = (struct dirent *(*)(void *)) readdir;
	while ((dp = (*readdirfunc)(dirp)) != NULL) {
		unsigned char *sc;
		Char *dc;

		if ((pglob->gl_flags & GLOB_LIMIT) &&
		    limit->l_readdir++ >= GLOB_LIMIT_READDIR) {
			errno = 0;
			*pathend++ = SEP;
			*pathend = EOS;
			error = GLOB_NOSPACE;
			break;
		}

		/*
		 * Initial DOT must be matched literally, unless we have
		 * GLOB_PERIOD set.
		 */
		if ((pglob->gl_flags & GLOB_PERIOD) == 0)
			if (dp->d_name[0] == DOT && *pattern != DOT)
				continue;
		/*
		 * If GLOB_NO_DOTDIRS is set, . and .. vanish.
		 */
		if ((pglob->gl_flags & GLOB_NO_DOTDIRS) &&
		    (dp->d_name[0] == DOT) &&
		    ((dp->d_name[1] == EOS) ||
		     ((dp->d_name[1] == DOT) && (dp->d_name[2] == EOS))))
			continue;
		/*
		 * The resulting string contains EOS, so we can
		 * use the pathlim character, if it is the nul
		 */
		for (sc = (unsigned char *) dp->d_name, dc = pathend; 
		     dc <= pathlim && (*dc++ = *sc++) != EOS;)
			continue;

		/*
		 * Have we filled the buffer without seeing EOS?
		 */
		if (dc > pathlim && *pathlim != EOS) {
			/*
			 * Abort when requested by caller, otherwise
			 * reset pathend back to last SEP and continue
			 * with next dir entry.
			 */
			if (pglob->gl_flags & GLOB_ERR) {
				error = GLOB_ABORTED;
				break;
			}
			else {
				*pathend = EOS;
				continue;
			}
		}

		if (globstar) {
#ifdef S_IFLNK
			if (!chase_symlinks &&
			    (g_lstat(pathbuf, &sbuf, pglob) ||
			    S_ISLNK(sbuf.st_mode)))
				continue;
#endif

			if (!match(pathend, pattern, termstar))
				continue;
	    
			if (--dc < pathlim - 2)
				*dc++ = SEP;
			*dc = EOS;
			error = glob2(pathbuf, dc, pathlim, pglobstar,
			    pglob, limit);
			if (error)
				break;
			*pathend = EOS;
		} else {
			if (!match(pathend, pattern, restpattern)) {
				*pathend = EOS;
				continue;
			}
			error = glob2(pathbuf, --dc, pathlim, restpattern,
			    pglob, limit);
			if (error)
				break;
		}
	}
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		(*pglob->gl_closedir)(dirp);
	else
		closedir(dirp);

	/*
	 * Again Posix X/Open issue with regards to error handling.
	 */
	if ((error || errno) && (pglob->gl_flags & GLOB_ERR))
		return GLOB_ABORTED;

	return error;
}


/*
 * Extend the gl_pathv member of a glob_t structure to accommodate a new item,
 * add the new item, and update gl_pathc.
 *
 * This assumes the BSD realloc, which only copies the block when its size
 * crosses a power-of-two boundary; for v7 realloc, this would cause quadratic
 * behavior.
 *
 * Return 0 if new item added, error code if memory couldn't be allocated.
 *
 * Invariant of the glob_t structure:
 *	Either gl_pathc is zero and gl_pathv is NULL; or gl_pathc > 0 and
 *	gl_pathv points to (gl_offs + gl_pathc + 1) items.
 */
static int
globextend(const Char *path, glob_t *pglob, struct glob_limit *limit)
{
	char **pathv;
	size_t i, newsize, len;
	char *copy;
	const Char *p;

	_DIAGASSERT(path != NULL);
	_DIAGASSERT(pglob != NULL);

	newsize = sizeof(*pathv) * (2 + pglob->gl_pathc + pglob->gl_offs);
	if ((pglob->gl_flags & GLOB_LIMIT) &&
	    newsize > GLOB_LIMIT_PATH * sizeof(*pathv))
		goto nospace;
	pathv = pglob->gl_pathv ? realloc(pglob->gl_pathv, newsize) :
	    malloc(newsize);
	if (pathv == NULL)
		return GLOB_NOSPACE;

	if (pglob->gl_pathv == NULL && pglob->gl_offs > 0) {
		/* first time around -- clear initial gl_offs items */
		pathv += pglob->gl_offs;
		for (i = pglob->gl_offs + 1; --i > 0; )
			*--pathv = NULL;
	}
	pglob->gl_pathv = pathv;

	for (p = path; *p++;)
		continue;
	len = (size_t)(p - path);
	limit->l_string += len;
	if ((copy = malloc(len)) != NULL) {
		if (g_Ctoc(path, copy, len)) {
			free(copy);
			return GLOB_ABORTED;
		}
		pathv[pglob->gl_offs + pglob->gl_pathc++] = copy;
	}
	pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

	if ((pglob->gl_flags & GLOB_LIMIT) &&
	    (newsize + limit->l_string) >= GLOB_LIMIT_STRING)
		goto nospace;

	return copy == NULL ? GLOB_NOSPACE : 0;
nospace:
	errno = 0;
	return GLOB_NOSPACE;
}


/*
 * pattern matching function for filenames.
 */
static int
match(const Char *name, const Char *pat, const Char *patend)
{
	int ok, negate_range;
	Char c, k;
	const Char *patNext, *nameNext, *nameStart, *nameEnd;

	_DIAGASSERT(name != NULL);
	_DIAGASSERT(pat != NULL);
	_DIAGASSERT(patend != NULL);
	patNext = pat;
	nameStart = nameNext = name;
	nameEnd = NULL;

	while (pat < patend || *name) {
		c = *pat;
		if (*name == EOS)
			nameEnd = name;
		switch (c & M_MASK) {
		case M_ALL:
			while ((pat[1] & M_MASK) == M_ALL) pat++;
			patNext = pat;
			nameNext = name + 1;
			pat++;
			continue;
		case M_ONE:
			if (*name == EOS)
				break;
			pat++;
			name++;
			continue;
		case M_SET:
			ok = 0;
			if ((k = *name) == EOS)
				break;
			pat++;
			name++;
			if ((negate_range = ((*pat & M_MASK) == M_NOT)) != EOS)
				++pat;
			while (((c = *pat++) & M_MASK) != M_END)
				if ((*pat & M_MASK) == M_RNG) {
					if (c <= k && k <= pat[1])
						ok = 1;
					pat += 2;
				} else if (c == k)
					ok = 1;
			if (ok == negate_range)
				break;
			continue;
		default:
			if (*name != c)
				break;
			pat++;
			name++;
			continue;
		}
		if (nameNext != nameStart
		    && (nameEnd == NULL || nameNext <= nameEnd)) {
			pat = patNext;
			name = nameNext;
			continue;
		}
		return 0;
	}
	return 1;
}

/* Free allocated data belonging to a glob_t structure. */
void
globfree(glob_t *pglob)
{
	size_t i;
	char **pp;

	_DIAGASSERT(pglob != NULL);

	if (pglob->gl_pathv != NULL) {
		pp = pglob->gl_pathv + pglob->gl_offs;
		for (i = pglob->gl_pathc; i--; ++pp)
			if (*pp)
				free(*pp);
		free(pglob->gl_pathv);
		pglob->gl_pathv = NULL;
		pglob->gl_pathc = 0;
	}
}

#ifndef __LIBC12_SOURCE__
int
glob_pattern_p(const char *pattern, int quote)
{
	int range = 0;

	for (; *pattern; pattern++)
		switch (*pattern) {
		case QUESTION:
		case STAR:
			return 1;

		case QUOTE:
			if (quote && pattern[1] != EOS)
			      ++pattern;
			break;

		case LBRACKET:
			range = 1;
			break;

		case RBRACKET:
			if (range)
			      return 1;
			break;
		default:
			break;
		}

	  return 0;
}
#endif

static DIR *
g_opendir(Char *str, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	_DIAGASSERT(str != NULL);
	_DIAGASSERT(pglob != NULL);

	if (!*str)
		(void)strlcpy(buf, ".", sizeof(buf));
	else {
		if (g_Ctoc(str, buf, sizeof(buf)))
			return NULL;
	}

	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return (*pglob->gl_opendir)(buf);

	return opendir(buf);
}

static int
g_lstat(Char *fn, __gl_stat_t *sb, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	_DIAGASSERT(fn != NULL);
	_DIAGASSERT(sb != NULL);
	_DIAGASSERT(pglob != NULL);

	if (g_Ctoc(fn, buf, sizeof(buf)))
		return -1;
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return (*pglob->gl_lstat)(buf, sb);
	return lstat(buf, sb);
}

static int
g_stat(Char *fn, __gl_stat_t *sb, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	_DIAGASSERT(fn != NULL);
	_DIAGASSERT(sb != NULL);
	_DIAGASSERT(pglob != NULL);

	if (g_Ctoc(fn, buf, sizeof(buf)))
		return -1;
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return (*pglob->gl_stat)(buf, sb);
	return stat(buf, sb);
}

static Char *
g_strchr(const Char *str, int ch)
{

	_DIAGASSERT(str != NULL);

	do {
		if (*str == ch)
			return __UNCONST(str);
	} while (*str++);
	return NULL;
}

static int
g_Ctoc(const Char *str, char *buf, size_t len)
{
	char *dc;

	_DIAGASSERT(str != NULL);
	_DIAGASSERT(buf != NULL);

	if (len == 0)
		return 1;

	for (dc = buf; len && (*dc++ = (char)(*str++)) != EOS; len--)
		continue;

	return len == 0;
}

#ifdef DEBUG
static void 
qprintf(const char *str, Char *s)
{
	Char *p;

	_DIAGASSERT(str != NULL);
	_DIAGASSERT(s != NULL);

	(void)printf("%s:\n", str);
	for (p = s; *p; p++)
		(void)printf("%c", CHAR(*p));
	(void)printf("\n");
	for (p = s; *p; p++)
		(void)printf("%c", *p & M_PROTECT ? '"' : ' ');
	(void)printf("\n");
	for (p = s; *p; p++)
		(void)printf("%c", ismeta(*p) ? '_' : ' ');
	(void)printf("\n");
}
#endif
