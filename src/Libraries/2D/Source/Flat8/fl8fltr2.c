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
 * $Source: n:/project/lib/src/2d/RCS/fl8fltr2.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:52:36 $
 *
 * Routine for scaling down by 2 with filtering onto a flat8 canvas
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8fltr2.c $
 * Revision 1.3  1993/10/19  09:52:36  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 *
 * Revision 1.2  1993/10/08  01:15:14  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 *
 * Revision 1.1  1993/08/05  20:06:53  jaemz
 * Initial revision
 */

#include "bitmap.h"
#include "cnvdat.h"
#include "scrdat.h"

/* Assumes bm is flat, assumes canvas is big enough to
   hold it, as it is, unclipped */
void flat8_filter2_ubitmap(grs_bitmap *bm) {
    grs_rgb a;
    uchar *c;

    uchar *dst = grd_bm.bits;
    uchar *src = bm->bits;

    int ds = 2 * bm->row - bm->w;
    int dd = grd_bm.row - grd_bm.w;

    /* variables
       a,src,dst,bm->row,grd_ipal,grd_bpal */

    /* Cycle through rows and do horizontal strips */
    for (int j = bm->h; j > 0; j -= 2) {
        for (int i = bm->w; i > 0; i -= 2) {
            a = (grd_bpal[*src] >> 2) & 0x3fc7f8ff;
            src++;
            a += (grd_bpal[*src] >> 2) & 0x3fc7f8ff;
            src += bm->row;
            a += (grd_bpal[*src] >> 2) & 0x3fc7f8ff;
            src--;
            a += (grd_bpal[*src] >> 2) & 0x3fc7f8ff;
            src -= bm->row;
            src += 2;

            c = grd_ipal;
            a = a >> 5;
            c += a & 0x1f;
            a = a >> 6;
            c += a & 0x3e0;
            a = a >> 6;
            c += a & 0x7c00;

            *dst = *c;
            dst++;
        }
        src += ds;
        dst += dd;
    }
}
