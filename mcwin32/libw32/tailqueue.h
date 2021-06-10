/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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

/*<<NaturalDoc>>

    Section:        Tail Queues
        Tail queue macros

    Description:
        A tail queue is headed by a pair of pointers, one to the head of the list and
        the other to the tail of the list. The elements are doubly linked so that an
        arbitrary element can be removed without a need to traverse the list. New
        elements can be added to the list after an existing element, at the head of the
        list, or at the end of the list. A tail queue may only be traversed in the
        forward direction.

    Topic:          Details
        Tail queues add the following functionality;

        1.  Entries can be added at the end of a list.

        However;

        1.  All list insertions and removals, except insertion before another element,
            must specify the head of the list.

        2.  Each head entry requires two pointers rather than one.

        3.  Code size is about 15% greater and operations run about 20% slower than lists.

    Usage:
        A tail queue is headed by a structure defined by the macro TAILQ_HEAD. This
        structure contains a pair of pointers, one to the first element in the tail
        queue and the other to the last element in the tail queue. The elements are
        doubly linked so that an arbitrary element can be removed without traversing
        the queue. New elements can be added to the queue after an existing element,
        before an existing element, at the head of the queue, or at the end of the queue.

        A structure is declared as follows:

>           TAILQ_HEAD(HEADNAME, TYPE) head;

        where is the name of the structure to be defined, and is the
        type of the elements to be linked into the tail queue. A pointer
        to the head of the tail queue can later be declared as:

>           struct HEADNAME *headp;

        Note:   The names head and headp are user selectable.

    Interface:
        A tail queue is headed by a structure defined by the TAILQ_HEAD macro. This
        structure contains a pair of pointers, one to the first element in the tail
        queue and the other to the last element in the tail queue. The elements are
        doubly linked so that an arbitrary element can be removed without traversing
        the tail queue. New elements can be added to the tail queue after an existing
        element, before an existing element, at the head of the tail queue, or at the
        end of the tail queue. A TAILQ_HEAD struc- ture is declared as follows:

>           TAILQ_HEAD(HEADNAME, TYPE) head;

        where HEADNAME is the name of the structure to be defined, and TYPE is the type
        of the elements to be linked into the tail queue. A pointer to the head of the
        tail queue can later be declared as:

>           struct HEADNAME *headp;

        (The names head and headp are user selectable.)

        The macro TAILQ_HEAD_INITIALIZER evaluates to an initializer for the tail queue
        head.

        The macro TAILQ_CONCAT concatenates the tail queue headed by head2 onto the end
        of the one headed by head1 removing all entries from the former.

        The macro TAILQ_EMPTY evaluates to true if there are no items on the tail queue.

        The macro TAILQ_ENTRY declares a structure that connects the elements in the
        tail queue.

        The macro TAILQ_FIRST returns the first item on the tail queue or NULL if the
        tail queue is empty.

        The macro TAILQ_FOREACH traverses the tail queue referenced by head in the
        forward direction, assigning each element in turn to var. var is set to NULL if
        the loop completes normally, or if there were no elements.

        The macro TAILQ_FOREACH_REVERSE traverses the tail queue referenced by head in
        the reverse direction, assigning each element in turn to var.

        The macros TAILQ_FOREACH_SAFE and TAILQ_FOREACH_REVERSE_SAFE traverse the list
        referenced by head in the forward or reverse direction respectively, assigning
        each element in turn to var. However, unlike their unsafe counterparts,
        TAILQ_FOREACH and TAILQ_FOREACH_REVERSE permit to both remove var as well as
        free it from within the loop safely without inter- fering with the traversal.

        The macro TAILQ_INIT initializes the tail queue referenced by head.

        The macro TAILQ_INSERT_HEAD inserts the new element elm at the head of the tail
        queue.

        The macro TAILQ_INSERT_TAIL inserts the new element elm at the end of the tail
        queue.

        The macro TAILQ_INSERT_AFTER inserts the new element elm after the element
        listelm.

        The macro TAILQ_INSERT_BEFORE inserts the new element elm before the element
        listelm.

        The macro TAILQ_LAST returns the last item on the tail queue. If the tail queue
        is empty the return value is NULL.

        The macro TAILQ_NEXT returns the next item on the tail queue, or NULL if this
        item is the last.

        The macro TAILQ_PREV returns the previous item on the tail queue, or NULL if
        this item is the first.

        The macro TAILQ_REMOVE removes the element elm from the tail queue.

    Example:

(start code)

        TAILQ_HEAD(tailq, entry) head;
        struct tailq * headp;                       // Queue head

    OR  typedef TAILQ_HEAD(tailq_t, entry) tailq_t;
        tailq_t * headp;

        struct entry {
            ...
            TAILQ_ENTRY(entry) node;                // Queue
            ...
        } *n1, *n2, *np;

        TAILQ_INIT(&head);                          // Initialize the tail queue

        n1 = malloc(sizeof(struct entry));          // Insert at the head
        TAILQ_INSHEAD(&head, entry, n1, node);

        n1 = malloc(sizeof(struct entry));          // Insert at the tail
        TAILQ_INSTAIL(&head, entry, n1, node);

        n2 = malloc(sizeof(struct entry));          // Insert after
        TAILQ_INSAFTER(&head, entry, n1, n2, node);

                                                    // Forward traversal.
        for (np = head.tqh_first; np; np = np->node.tqe_next)
            np-> ...

    OR  for (np = TAILQ_HEAD(&head); np != TAILQ_END>(&head);
                np = TAILQ_NEXT(np, node))
            np-> ...

        while (np = TAILQ_HEAD(&head))              // Delete.
            TAILQ_REMOVE(&head, entry, np);

(end)

    History:
        The queue functions first appeared in 4.4BSD.

*/

/* Note that the all of the following header is idempotent except the
 * following section, which is explicitly required not to be idempotent.
 * Therefore, it is by intent that the header guards
 * (#ifndef TAILQUEUE_H_INCLUDED) do not span this entire file.
 */

#undef  __TAILQTYPE                 /* tailq macro 'typename' */
#undef  __TAILQNUL
#if defined(__cplusplus) && defined(TAILQ_CLASS)
#define __TAILQTYPE     class
#define __TAILQNUL      0
#else
#define __TAILQTYPE     struct
#define __TAILQNUL      NULL
#endif

#ifndef GR_TAILQUEUE_H_INCLUDED
#define GR_TAILQUEUE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_tailqueue_h,"$Id: tailqueue.h,v 1.1 2021/06/10 15:45:04 cvsuser Exp $")

/*
 * Tail queue declarations.
 */
#define TAILQ_HEAD(name, type) \
__TAILQTYPE name { \
    struct type *tqh_first;         /* first element */ \
    struct type **tqh_last;         /* addr of last next element */ \
}

#define TAILQ_HEAD_INITIALIZER(head) \
    { __TAILQNUL, &(head).tqh_first }

#define TAILQ_ENTRY(type) \
__TAILQTYPE { \
    struct type *tqe_next;          /* next element */ \
    struct type **tqe_prev;         /* address of previous next element */ \
}

/*
 * Tail queue functions.
 */
#define TAILQ_CONCAT(head1, head2, field) do { \
    if (!TAILQ_EMPTY(head2)) { \
        *(head1)->tqh_last = (head2)->tqh_first; \
        (head2)->tqh_first->field.tqe_prev = (head1)->tqh_last; \
        (head1)->tqh_last = (head2)->tqh_last; \
        TAILQ_INIT((head2)); \
    } \
} while (0)

#define TAILQ_EMPTY(head) ((head)->tqh_first == __TAILQNUL)

#define TAILQ_FIRST(head) ((head)->tqh_first)

#define TAILQ_FOREACH(var, head, field) \
    for ((var) = TAILQ_FIRST((head)); \
        (var); \
        (var) = TAILQ_NEXT((var), field))

#define TAILQ_FOREACH_REVERSE(var, head, headname, field) \
    for ((var) = TAILQ_LAST((head), headname); \
        (var); \
        (var) = TAILQ_PREV((var), headname, field))

#define TAILQ_INIT(head) do { \
    TAILQ_FIRST((head)) = __TAILQNUL; \
    (head)->tqh_last = &TAILQ_FIRST((head)); \
} while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, field) do { \
    if ((TAILQ_NEXT((elm), field) = TAILQ_NEXT((listelm), field)) != __TAILQNUL) \
        TAILQ_NEXT((elm), field)->field.tqe_prev = \
            &TAILQ_NEXT((elm), field); \
    else { \
        (head)->tqh_last = &TAILQ_NEXT((elm), field); \
    } \
    TAILQ_NEXT((listelm), field) = (elm); \
    (elm)->field.tqe_prev = &TAILQ_NEXT((listelm), field); \
} while (0)

#define TAILQ_INSERT_BEFORE(listelm, elm, field) do { \
    (elm)->field.tqe_prev = (listelm)->field.tqe_prev; \
    TAILQ_NEXT((elm), field) = (listelm); \
    *(listelm)->field.tqe_prev = (elm); \
    (listelm)->field.tqe_prev = &TAILQ_NEXT((elm), field); \
} while (0)

#define TAILQ_INSERT_HEAD(head, elm, field) do { \
    if ((TAILQ_NEXT((elm), field) = TAILQ_FIRST((head))) != __TAILQNUL) \
        TAILQ_FIRST((head))->field.tqe_prev = \
            &TAILQ_NEXT((elm), field); \
    else \
        (head)->tqh_last = &TAILQ_NEXT((elm), field); \
    TAILQ_FIRST((head)) = (elm); \
    (elm)->field.tqe_prev = &TAILQ_FIRST((head)); \
} while (0)

#define TAILQ_INSERT_TAIL(head, elm, field) do { \
    TAILQ_NEXT((elm), field) = __TAILQNUL; \
    (elm)->field.tqe_prev = (head)->tqh_last; \
    *(head)->tqh_last = (elm); \
    (head)->tqh_last = &TAILQ_NEXT((elm), field); \
} while (0)

#define TAILQ_LAST(head, headname) \
    (*(((struct headname *)((head)->tqh_last))->tqh_last))

#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define TAILQ_PREV(elm, headname, field) \
    (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define TAILQ_REMOVE(head, elm, field) do { \
    if ((TAILQ_NEXT((elm), field)) != __TAILQNUL) \
        TAILQ_NEXT((elm), field)->field.tqe_prev = \
            (elm)->field.tqe_prev; \
    else { \
        (head)->tqh_last = (elm)->field.tqe_prev; \
    } \
    *(elm)->field.tqe_prev = TAILQ_NEXT((elm), field); \
} while (0)

#endif /*GR_TAILQUEUE_H_INCLUDED*/
