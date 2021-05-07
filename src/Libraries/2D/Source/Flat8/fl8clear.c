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
 * $Source: n:/project/lib/src/2d/RCS/fl8clear.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:18 $
 *
 * Routines for clearing a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8clear.c $
 * Revision 1.3  1993/10/19  09:50:18  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 *
 * Revision 1.2  1993/10/08  01:15:08  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 *
 * Revision 1.1  1993/02/16  14:14:00  kaboom
 * Initial revision
 */

#include <string.h>

#include "cnvdat.h"
#include "lg.h"

/**
 * Clear FLAT8 Canvas, current in context
 * @param color filling color from palette
 */
void flat8_clear(int32_t color) {
    color &= 0x00ff;

    uint8_t *p = grd_bm.bits;
    int h = grd_bm.h;
    int w = grd_bm.w;
    int row = grd_bm.row;

    while (h--) {
        // MLA - inlined this code
        memset(p, color, w);
        p += row; // FIXME: Why there row, not w? On some images row != w, why?
    }
}
