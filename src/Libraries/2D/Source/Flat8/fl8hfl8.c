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
/*
 * $Source: n:/project/lib/src/2d/RCS/fl8hfl8.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:33 $
 *
 * flat 8 bitmap horizontal flip routine.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8hfl8.c $
 * Revision 1.3  1993/10/19  09:50:33  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 *
 * Revision 1.2  1993/10/08  01:15:16  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 *
 * Revision 1.1  1993/06/06  15:09:08  kaboom
 * Initial revision
 */

#include "cnvdat.h"

/* draw an unclipped, horizontally flipped flat 8 bitmap to a flat 8
   canvas. */
void flat8_hflip_flat8_ubitmap(grs_bitmap *bm, short x, short y) {
    ushort brow = bm->row;
    ushort grow = grd_bm.row;
    short bw = bm->w;

    short h = bm->h;                                  /* height */
    uchar *src = bm->bits;                            /* pointer into source bitmap */
    uchar *dst = grd_bm.bits + y * grow + x + bw - 1; /* pointer into canvas memory */
    while (h--) {
        short w = bw; /* bitmap width */
        while (w--)
            *dst-- = *src++;
        src += brow - bw;
        dst += grow + bw;
    }
}
