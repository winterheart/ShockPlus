/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef PQUEUE_H
#define PQUEUE_H

/*
 * $Source: n:/project/lib/src/dstruct/RCS/pqueue.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1993/08/09 20:31:11 $
 *
 * $Log: pqueue.h $
 * Revision 1.1  1993/08/09  20:31:11  mahk
 * Initial revision
 *
 *
 */

// -----------------------------------
// Priority Queue Abstraction
// -----------------------------------
/* Herein lies a binary heap implementation of a priority queue
   The queue can have elements of any size, as the client specifies
   the element size and comparison function.  */

// Includes
#include <stdio.h>
#include <stdint.h>

#include "lg.h" // every file should have this
#include "lg_error.h"

#ifdef __cplusplus
extern "C" {
#endif

// Defines

// Comparison function, works like strcmp
typedef int (*QueueCompare)(void *elem1, void *elem2);

#pragma pack(push, 2)
typedef struct _pqueue {
    int32_t size;
    int32_t fullness;
    int32_t elemsize;
    uchar grow;
    char *vec;
    QueueCompare comp;
} PQueue;

// Prototypes

// Initializes a Priority queue to a particular size, with a
// particular element size and comparison function.
errtype pqueue_init(PQueue *q, int size, int elemsize, QueueCompare comp, uchar grow);

// Insert an element into the queue (log time)
errtype pqueue_insert(PQueue *q, void *elem);

// Copies the least element in the queue into *elem,
// and removes that element. (log time)
errtype pqueue_extract(PQueue *q, void *elem);

// Copies the least element into *elem, but does not remove it.  (constant time)
errtype pqueue_least(PQueue *q, void *elem);

// Writes out a queue to file number fd, calling writefunc to write out each element.
// If writefunc is NULL, simply writes the literal data in each element.
errtype pqueue_write(PQueue *q, FILE *fd, void (*writefunc)(FILE *fd, void *elem));

// Reads in a queue from file number fd, calling readfunc to read each element.
// If readfunc is NULL, reads each element literally.
errtype pqueue_read(PQueue *q, FILE *fd, void (*readfunc)(FILE *fd, void *elem));

// Destroys a priority queue.
errtype pqueue_destroy(PQueue *q);

// Globals

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // PQUEUE_H
