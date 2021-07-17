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
//
// $Source: r:/prj/lib/src/2d/RCS/fl8dbl.asm $
// $Revision: 1.3 $
// $Author: kevin $
// $Date: 1994/09/08 00:00:18 $
//
// Bitmap doubling primitives.
//

#include "blndat.h"
#include "grs.h"
#include "lg.h"

// ------------------------------------------------------------------------
// PowerPC routines
// ------------------------------------------------------------------------

// ========================================================================
// src = eax, dest = edx
void flat8_flat8_smooth_hv_double_ubitmap(grs_bitmap *src, grs_bitmap *dst) {
    uchar *srcPtr, *dstPtr;

    dstPtr = dst->bits;
    srcPtr = src->bits;

    // HAX HAX HAX no smooth doubling for now!
    for (int y = 0; y < src->h; y++) {
        for (int x = 0; x < src->w; x++) {
            *(dstPtr) = *srcPtr;
            *(dstPtr + 1) = *srcPtr;
            *(dstPtr + src->w * 2) = *srcPtr;
            *((dstPtr + src->w * 2) + 1) = *srcPtr;
            dstPtr += 2;
            srcPtr++;
        }
        dstPtr += src->w * 2;
    }
}
