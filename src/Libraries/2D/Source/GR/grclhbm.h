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
 * $Source: n:/project/lib/src/2d/RCS/grclhbm.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/10/08 01:15:56 $
 * Dispatch macros for clut horizontal bitmap flip routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: grclhbm.h $
 * Revision 1.2  1993/10/08  01:15:56  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 *
 * Revision 1.1  1993/10/06  13:50:54  kevin
 * Initial revision
 *
 */

#ifndef __GRCLHBM_H
#define __GRCLHBM_H
#include "icanvas.h"

#define gr_clut_hflip_flat8_ubitmap \
    ((void (*)(grs_bitmap * bm, short x, short y, uchar *cl)) grd_canvas_table[CLUT_HFLIP_FLAT8_UBITMAP])

#endif /* !__GRCLHBM_H */
