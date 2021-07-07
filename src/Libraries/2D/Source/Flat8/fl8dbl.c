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

// ========================================================================
void flat8_flat8_smooth_h_double_ubitmap(grs_bitmap *srcb, grs_bitmap *dstb) {
    uchar *src = srcb->bits, *dst = dstb->bits;
    ushort curpix, tempshort;
    uchar *local_grd_half_blend;

    local_grd_half_blend = grd_half_blend;
    if (!local_grd_half_blend)
        return;

    long srcAdd = (srcb->row - srcb->w) - 1;
    long dstAdd = dstb->row - (srcb->w << 1);
    int endh = srcb->w - 1;
    int endv = srcb->h;

    for (int v = 0; v < endv; v++) {
        curpix = *(short *)src;
        src += 2;
        for (int h = 0; h < endh; h++) {
            tempshort = curpix & 0xff00;
            tempshort |= local_grd_half_blend[curpix];
            *(ushort *)dst = tempshort;
            dst += 2;
            curpix = (curpix << 8) | *(src++);
        }

        // double last pixel
        curpix >>= 8;
        *(dst++) = curpix;
        *(dst++) = curpix;

        src += srcAdd;
        dst += dstAdd;
    }
}

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
