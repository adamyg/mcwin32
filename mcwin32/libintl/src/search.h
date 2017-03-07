/* $NetBSD: search.h,v 1.19 2011/09/14 23:34:26 christos Exp $ */

/*
 * Written by J.T. Conklin <jtc@NetBSD.org>
 * Public domain.
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <sys/cdefs.h>

typedef struct entry {
	char *key;
	void *data;
} ENTRY;

#ifdef _SEARCH_PRIVATE
typedef struct node {
        char *key;
	struct node *llink, *rlink;
} node_t;
#endif

__BEGIN_DECLS
void	*tfind(const void *, void * const *, int (*)(const void *, const void *));
void	*tsearch(const void *, void **, int (*)(const void *, const void *));
__END_DECLS

#endif  /*!_SEARCH_H_*/


