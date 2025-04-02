
   UNIX manpage compiler:

        The mandoc UNIX manpage compiler toolset (http://mdocml.bsd.lv/)

        Current version 1.14.6 (23 Sept 2021) using sqlite3 (version 3.25.2)
        Version 1.14.5 (March 10, 2019) using sqlite3 (version 3.25.2)
        Version 1.14.4 (August 8, 2018) using sqlite3 (version 3.25.2)
        Version 1.13.4 (July 10, 2016) using sqlite3 (version 3.16.2)
        Version 1.12.3 (December 31, 2013)

        mandoc is a suite of tools compiling mdoc, the roff macro
        language of choice for BSD manual pages, and man, the
        predominant historical language for UNIX manuals. It is small,
        ISO C, ISC-licensed, and quite fast.

        The main component of the toolset is the mandoc utility
        program, based on the libmandoc validating compiler, to
        format output for UNIX terminals (with support for
        wide-character locales), XHTML, HTML, PostScript, and PDF.

        mandoc has predominantly been developed on OpenBSD and is
        both an OpenBSD and a BSD.lv project. We strive to support
        all interested free operating systems, in particular
        DragonFly, NetBSD, FreeBSD, Minix 3, and GNU/Linux, as well
        as all systems running the pkgsrc portable package build
        system.

   License:

        With the exceptions noted below, all code and documentation contained in the
        mandoc toolkit is protected by the Copyright of the following developers:

        Copyright (c) 2008-2012, 2014 Kristaps Dzonsons <kristaps@bsd.lv>
        Copyright (c) 2010-2021 Ingo Schwarze <schwarze@openbsd.org>
        Copyright (c) 1999, 2004, 2017 Marc Espie <espie@openbsd.org>
        Copyright (c) 2009, 2010, 2011, 2012 Joerg Sonnenberger <joerg@netbsd.org>
        Copyright (c) 2013 Franco Fichtner <franco@lastsummer.de>
        Copyright (c) 2014 Baptiste Daroussin <bapt@freebsd.org>
        Copyright (c) 2016 Ed Maste <emaste@freebsd.org>
        Copyright (c) 2017 Michael Stapelberg <stapelberg@debian.org>
        Copyright (c) 2017 Anthony Bentley <bentley@openbsd.org>
        Copyright (c) 1998, 2004, 2010, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
        Copyright (c) 2008, 2017 Otto Moerbeek <otto@drijf.net>
        Copyright (c) 2004 Ted Unangst <tedu@openbsd.org>
        Copyright (c) 1994 Christos Zoulas <christos@netbsd.org>
        Copyright (c) 2003, 2007, 2008, 2014 Jason McIntyre <jmc@openbsd.org>

        See the individual source files for information about who contributed
        to which file during which years.


        The mandoc distribution as a whole is distributed by its developers
        under the following license:

        Permission to use, copy, modify, and distribute this software for any
        purpose with or without fee is hereby granted, provided that the above
        copyright notice and this permission notice appear in all copies.

        THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
        WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
        MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
        ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
        WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
        ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
        OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

        The following files included from outside sources are protected by
        other people's Copyright and are distributed under various 2-clause
        and 3-clause BSD licenses; see these individual files for details.

        soelim.c, soelim.1:
        Copyright (c) 2014 Baptiste Daroussin <bapt@freebsd.org>

        compat_err.c, compat_fts.c, compat_fts.h,
        compat_getsubopt.c, compat_strcasestr.c, compat_strsep.c,
        man.1:
        Copyright (c) 1989,1990,1993,1994 The Regents of the University of California

        compat_stringlist.c, compat_stringlist.h:

   Build Notes:

       o Under Cygwin the use of '-liberty' can result in getopt() crashes since
           both liberty and libc have incompatible definitions of optarg.

         Either remove liberty from the generated Makefile link line and/or
         remove getopt() from your local liberty image.

>          ar d /usr/lib/libiberty.a getopt.o getopt1.o

       o pdf support requires a c99 compliant snprintf() implementation,
           for example "%zn".

===


