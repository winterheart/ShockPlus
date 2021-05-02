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
 * $Source: r:/prj/lib/src/2d/RCS/permap.c $
 * $Revision: 1.8 $
 * $Author: kevin $
 * $Date: 1994/12/01 14:59:57 $
 *
 * Full perspective texture mapping dispatchers.
 *
 */

#include "bitmap.h"
#include "buffer.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "cnvdat.h"
#include "fill.h"
#include "fl8p.h"
#include "ifcn.h"
#include "grnull.h"
#include "pertyp.h"
#include "scrmac.h"
#include "tmapfcn.h"
#include "tmaps.h"
#include "tmaptab.h"

extern int gri_per_umap_setup(int n, grs_vertex **vpl, grs_per_setup *ps);

uchar grd_enable_quad_blend = FALSE;

int per_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti) {
    grs_vertex **cpl; /* clipped vertices */
    int m;            /* number of clipped vertices */

    cpl = NULL;
    m = gr_clip_poly(n, 5, vpl, &cpl);
    if (m > 2)
        per_umap(bm, m, cpl, ti);
    gr_free_temp(cpl);

    return ((m > 2) ? CLIP_NONE : CLIP_ALL);
}

void per_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti) {
    short percode;
    grs_per_setup ps;
    uchar *save_bits;

    ps.dp = bm->flags & BMF_TRANS;
    if (2 * grd_gc.fill_type + ps.dp == 2 * FILL_SOLID) {
        h_umap(bm, n, vpl, ti);
        return;
    }
    percode = gri_per_umap_setup(n, vpl, &ps);

    /* should be set by init func, but just in case...*/
    ps.shell_func = gr_null;

    ps.dp += ti->tmap_type + (GRD_FUNCS * bm->type);
    if (grd_gc.fill_type != FILL_NORM)
        ps.fill_parm = grd_gc.fill_parm;
    else if (ti->flags & TMF_CLUT)
        if ((ps.clut = ti->clut) == NULL)
            ps.clut = gr_get_clut();

    save_bits = bm->bits; /* in case bitmap type is rsd8 */

    switch (percode) {
    case GR_PER_CODE_BIGSLOPE:
        ((void (*)(grs_bitmap *, grs_per_setup *))(grd_tmap_hscan_init_table[ps.dp]))(bm, &ps);
        ((void (*)(grs_bitmap *, int, grs_vertex **, grs_per_setup *))(ps.shell_func))(bm, n, vpl, &ps);
        break;
    case GR_PER_CODE_SMALLSLOPE:
        ((void (*)(grs_bitmap *, grs_per_setup *))(grd_tmap_vscan_init_table[ps.dp]))(bm, &ps);
        ((void (*)(grs_bitmap *, int, grs_vertex **, grs_per_setup *))(ps.shell_func))(bm, n, vpl, &ps);
        break;
    case GR_PER_CODE_LIN:
        ti->tmap_type += GRC_BILIN - GRC_PER;
        h_umap(bm, n, vpl, ti);
        break;
    case GR_PER_CODE_FLOOR:
        ti->tmap_type += GRC_FLOOR - GRC_PER;
        ti->flags |= TMF_FLOOR;
        h_umap(bm, n, vpl, ti);
        break;
    case GR_PER_CODE_WALL:
        ti->tmap_type += GRC_WALL2D - GRC_PER;
        ti->flags |= TMF_WALL;
        v_umap(bm, n, vpl, ti);
        break;
    }
    bm->bits = save_bits;
}
