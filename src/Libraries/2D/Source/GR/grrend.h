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
 * $Source: r:/prj/lib/src/2d/RCS/grrend.h $
 * $Revision: 1.14 $
 * $Author: kevin $
 * $Date: 1994/08/04 09:46:09 $
 *
 * Dispatch macros for rendering functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: grrend.h $
 * Revision 1.14  1994/08/04  09:46:09  kevin
 * Added new wire poly line functionality.
 *
 * Revision 1.13  1994/06/11  00:35:40  lmfeeney
 * lines removed from canvas table, #defines for backward compatibility
 *
 * Revision 1.12  1993/10/26  02:17:43  kevin
 * Changed gr_scale... and gr_clut_scale... dispatch macros
 * so that they may be used with any bitmap type.
 * (rsd8 and flat8 types are currently supported.)
 *
 * Revision 1.11  1993/10/20  15:22:00  kaboom
 * Took out spoly dispatch macros.
 *
 * Revision 1.10  1993/10/19  10:22:15  kaboom
 * Moved solid and color shaded (but not intensity shaded) polygon macros
 * to other files.
 *
 * Revision 1.9  1993/10/02  01:01:41  kaboom
 * Moved texture map stuff to other files.
 *
 * Revision 1.8  1993/09/02  20:12:06  kaboom
 * Added macros for dispatching of gr_lin_{u}map by bitmap type.  Also
 * put in macros for bypass 8- and 24-bit linmaps.
 *
 * Revision 1.7  1993/08/19  21:51:21  jaemz
 * Added bitmap scale clut and voxel functions
 *
 * Revision 1.6  1993/08/10  19:08:12  kaboom
 * Added macros for gr_lit_{u}tmap()
 *
 * Revision 1.5  1993/07/20  16:24:54  jaemz
 * Added interp2 and filterw2
 *
 * Revision 1.4  1993/06/22  19:59:50  spaz
 * Added macros for goroud-shadedl ines:
 * gr_fix_usline, sline, ucline, and cline. (fix only)
 *
 * Revision 1.3  1993/06/14  14:11:33  kaboom
 * Added macros for new lin_{u}map and lin_lit_lin_{u}map routines.
 *
 * Revision 1.2  1993/06/01  13:51:25  kaboom
 * Added macros for lit perspective tmappers.
 *
 * Revision 1.1  1993/04/29  18:36:58  kaboom
 * Initial revision
 */

#ifndef __GRREND_H
#define __GRREND_H

#include "grs.h"
#include "icanvas.h"
#include "tabdat.h"

#include "ctxmac.h"
#include "rgb.h"
#include "grlin.h"

/* compatibility -- rah ! */

/* - Lines have been eliminated from the canvas table.
   - The preferred interface now takes color and fill parameters and
   either a grs_vertex structure or short points.

   Unclipped line routines have been inlined to convert parameters and
   call the new function via the uline_fill table.

   Clipped line routines return a value and so cannot be macro inlined
   this way.  The canvas lookup macro is now a function which is
   extern'ed by it's old 'gen' name.

   Again: the preferred interface is through the new parameter'ed functions
   in the uline_fill and line_clip_fill tables.

*/

/* lines */

#define gr_fix_line gen_fix_line
extern int gen_fix_line(fix x0, fix y0, fix x1, fix y1);

/* rgb shaded lines */

extern int gen_fix_cline(fix x0, fix y0, grs_rgb c0, fix x1, fix y1, grs_rgb c1);

/* i shaded lines */

extern int gen_fix_sline(fix x0, fix y0, fix i0, fix x1, fix y1, fix i1);

/* These are horrific: something should be done. */
#define gr_scale_ubitmap(bm, x, y, w, h)                                  \
    ((void (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h)) \
         grd_canvas_table[SCALE_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, y, w, h)
#define gr_scale_bitmap(bm, x, y, w, h)                                  \
    ((int (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h)) \
         grd_canvas_table[SCALE_DEVICE_BITMAP + 2 * ((bm)->type)])(bm, x, y, w, h)

#define gr_clut_scale_ubitmap(bm, x, y, w, h, cl)                                    \
    ((int (*)(grs_bitmap * _bm, short _x, short _y, short _w, short _h, uchar *_cl)) \
         grd_canvas_table[CLUT_SCALE_DEVICE_UBITMAP + 2 * ((bm)->type)])(bm, x, y, w, h, cl)

#endif /* !__GRREND_H */
