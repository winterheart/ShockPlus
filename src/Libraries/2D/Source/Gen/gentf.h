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
 * $Source: r:/prj/lib/src/2d/RCS/gentf.h $
 * $Revision: 1.9 $
 * $Author: kevin $
 * $Date: 1994/10/25 16:52:11 $
 *
 * Texture mapping internal functions.
 * Generic canvas.
 *
 */
#include "pertyp.h"
#include "tmapint.h"

/* bitmap blitters */
extern int gen_flat8_bitmap(grs_bitmap *bm, short x, short y);

/* rsd8 */
extern void gri_gen_rsd8_ubitmap(grs_bitmap *bm, short x, short y);
extern int gri_gen_rsd8_bitmap(grs_bitmap *bm, short x_left, short y_top);

extern void rsd8_tm_init(grs_tmap_loop_info *ti);
extern void rsd8_pm_init(grs_bitmap *bm, grs_per_setup *ps);

