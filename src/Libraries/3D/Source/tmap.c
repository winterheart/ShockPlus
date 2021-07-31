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
// $Source: r:/prj/lib/src/3d/RCS/tmap.asm $
// $Revision: 1.30 $
// $Author: jaemz $
// $Date: 1994/09/06 00:12:15 $
//
// Texture mappers
//
// $Log: tmap.asm $
// Revision 1.30  1994/09/06  00:12:15  jaemz
// Externed vbuf2 and nvert so we can peek at them later
//
// Revision 1.29  1994/08/18  03:47:36  jaemz
// added underscore for c to stereo globvs
//
// Revision 1.28  1994/08/16  18:37:43  kevin
// moved constants to 2d.inc.  Modified code for compatability with new 2d.
//
// Revision 1.27  1994/08/04  16:36:36  jaemz
// *** empty log message ***
//
// Revision 1.26  1994/07/21  22:44:46  jaemz
// arrrgh
//
// Revision 1.25  1994/07/19  13:48:47  jaemz
// Added support for stereo
//
// Revision 1.24  1994/07/06  22:37:14  kevin
// use per_umap directly instead of going through canvas table.
//
// Revision 1.23  1994/05/19  09:41:20  kevin
// g3_light(draw)_t(l,floor_,wall_)map now use watcom register passing
// conventions. All perspective maps pass linearity tests.  Non/power/of/2
// bitmaps are punted (for now). Point copy loop has been somewhat optimized.
//
// Revision 1.22  1994/05/04  18:46:44  kevin
// Do clut lighting check correctly.  Check floors and full perspective
// tmaps for linearity.
//
// Revision 1.21  1994/05/02  23:00:52  kevin
// Added support for wall and floor specific texture maps.
//
// Revision 1.20  1993/12/27  21:29:32  kevin
// Hacked in rsd support.
//
// Revision 1.19  1993/12/04  12:44:22  kevin
// Changed 2d calls to conform to new icanvas.inc.
//
// Revision 1.18  1993/10/25  16:28:48  kaboom
// Same bug as last time for g3_draw_tmap.
//
// Revision 1.17  1993/10/22  12:58:02  kaboom
// Fixed bug in quad_tile routines---was losing pointer to vertices.
//
// Revision 1.16  1993/10/22  09:35:26  kaboom
// Added new linear map routines.
//
// Revision 1.15  1993/10/13  13:38:05  kevin
// Changed copy_loop to scale to full bitmap; hopefully without causing
// wrapping.
//
// Revision 1.14  1993/10/03  10:20:30  kaboom
// Fixed 2 register saving bugs.  Also cleanup up code a little.
//
// Revision 1.13  1993/10/02  11:02:07  kaboom
// Changed names of clip_{line,polygon} to g3_clip_{line,polygon} to avoid
// name collisions.
//
// Revision 1.12  1993/10/02  09:29:54  kaboom
// Now calls uv versions of 2d primitives.
//
// Revision 1.11  1993/08/11  15:02:14  kaboom
// Added support for lighting texture mappers.
//
// Revision 1.10  1993/08/10  22:54:27  dc
// add _3d.inc to includes
//
// Revision 1.9  1993/07/08  23:37:54  kaboom
// Changed old use of align field in grs_bitmap to new wlog and hlog.
//
// Revision 1.8  1993/06/16  14:05:06  kaboom
// Replaced code for tiling in calc_warp_matrix accidentally deleted in
// last revision.
//
// Revision 1.7  1993/06/15  19:00:49  kaboom
// Fixed overflow and scaling problems with warp matrix calculation.
//
// Revision 1.6  1993/06/09  15:34:29  kaboom
// Fixed non-initialization of codes for g3_draw_tmap_tile, which
// sometimes caused massive destruction.
//
// Revision 1.5  1993/06/09  04:24:25  kaboom
// Changed g3_draw_tmap_tile to take basis vectors instead of texture map
// width and height.
//
// Revision 1.4  1993/05/11  15:24:21  matt
// Changed g3_draw_tmap_tile() to reuse warp matrix if anchor==0
//
// Revision 1.3  1993/05/11  15:02:00  matt
// Fixed an overflow problem in the warp matrix setup
//
// Revision 1.2  1993/05/10  15:07:51  matt
// Fixed g3_draw_tmap_quad_tile(), which didn't do divide for width and height
// count, so always acted like w,h=1,1.
//
// Revision 1.1  1993/05/04  17:39:54  matt
// Initial revision
//

#include "3d.h"
#include "clip.h"
#include "fl8p.h"
#include "globalv.h"
#include "lg.h"
#include "polygon.h"
#include "tmapfcn.h"

// prototypes
int do_tmap(int n, g3s_phandle *vp, grs_bitmap *bm);
int draw_tmap_common(int n, g3s_phandle *vp, grs_bitmap *bm);
int check_linear(int n);

void *tmap_func;
grs_bitmap unpack_bm;
grs_tmap_info ti;
grs_tmap_info *ti_ptr = &ti;

long light_flag;

// takes eax=nverts edx=ptr to points, ebx=ptr to bitmap
int g3_draw_floor_map(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&h_umap;
    ti.tmap_type = GRC_FLOOR;
    ti.flags = TMF_FLOOR;
    light_flag = 0;
    return (do_tmap(n, vp, bm));
}

int g3_light_floor_map(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&h_umap;
    ti.tmap_type = GRC_LIT_FLOOR;
    ti.flags = TMF_FLOOR;
    light_flag = 1;
    return (do_tmap(n, vp, bm));
}

int g3_draw_wall_map(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&v_umap;
    ti.tmap_type = GRC_WALL1D;
    ti.flags = TMF_WALL;
    light_flag = 0;
    return (do_tmap(n, vp, bm));
}

int g3_light_wall_map(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&v_umap;
    ti.tmap_type = GRC_LIT_WALL1D;
    ti.flags = TMF_WALL;
    light_flag = 1;
    return (do_tmap(n, vp, bm));
}

int g3_draw_lmap(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&h_umap;
    ti.tmap_type = GRC_BILIN;
    ti.flags = 0;
    light_flag = 0;
    return (do_tmap(n, vp, bm));
}

int g3_light_lmap(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&h_umap;
    ti.tmap_type = GRC_LIT_BILIN;
    ti.flags = 0;
    light_flag = 1;
    return (do_tmap(n, vp, bm));
}

int g3_light_tmap(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&per_umap;
    ti.tmap_type = GRC_LIT_PER;
    ti.flags = 0;
    light_flag = 1;
    return (do_tmap(n, vp, bm));
}

int g3_draw_tmap(int n, g3s_phandle *vp, grs_bitmap *bm) {
    tmap_func = (void *)&per_umap;
    ti.tmap_type = GRC_PER;
    ti.flags = 0;
    light_flag = 0;
    return (do_tmap(n, vp, bm));
}

int do_tmap(int n, g3s_phandle *vp, grs_bitmap *bm) {
    // convert RSD bitmap to normal
    if (bm->type == BMT_RSD8) {
        if (gr_rsd8_convert(bm, &unpack_bm) != GR_UNPACK_RSD8_OK)
            return CLIP_ALL;
        else
            bm = &unpack_bm;
    }

    // get codes for this polygon
    byte andcode = 0xff;
    byte orcode = 0;
    g3s_phandle *src = vp;
    for (int i = n; i > 0; i--) {
        g3s_phandle tempHand = *(src++);
        andcode &= tempHand->codes;
        orcode |= tempHand->codes;
    }

    // check codes for trivial reject.
    if (andcode)
        return CLIP_ALL;

    return (draw_tmap_common(n, vp, bm));
}

int draw_tmap_common(int n, g3s_phandle *vp, grs_bitmap *bm) {
    int branch_to_copy;
    g3s_phandle temphand;
    int i, temp;
    grs_vertex *cur_vert;

    // always clip for now
    // copy to temp buffer for clipping
    // BlockMove(vp,vbuf,n*4);
    memmove(vbuf, vp, n * sizeof *vbuf);

    _n_verts = n = g3_clip_polygon(n, vbuf, vbuf2);
    if (n == 0)
        return CLIP_ALL;
    if (ti.tmap_type >= GRC_CLUT_BILIN + 2)
        branch_to_copy = check_linear(n);
    else
        branch_to_copy = 1;

    // check if bitmap is power of 2
    int wlog = bm->wlog;
    int hlog = bm->hlog;

    if (((1 << wlog) != bm->w) || ((1 << hlog) != bm->h))
        return CLIP_ALL;

    wlog += 8;
    hlog += 8;

    // now, copy 2d points to buffer for tmap call, projecting if neccesary
    //	for (i=n-1; i--; i>=0)
    for (i = 0; i < n; i++) {
        temphand = vbuf2[i];
        p_vpl[i] = cur_vert = &p_vlist[i];

        // check if this point has been projected
        if ((temphand->p3_flags & PF_PROJECTED) == 0)
            g3_project_point(temphand);

        cur_vert->x = temphand->sx;
        cur_vert->y = temphand->sy;

        temp = temphand->uv.u;
        if (temp <= 0)
            temp++;
        else
            temp--;
        cur_vert->u = temp << wlog;

        temp = temphand->uv.v;
        if (temp <= 0)
            temp++;
        else
            temp--;
        cur_vert->v = temp << hlog;

        cur_vert->i = temphand->i << 8;

        if (!branch_to_copy) // fix Z
        {
            temp = temphand->gZ;
            cur_vert->w = fix_div(0x010000, temp); // 1/Z
        }
    }

    if (!light_flag) {
        ((void (*)(grs_bitmap * bm, int n, grs_vertex **vpl, grs_tmap_info *ti)) tmap_func)(bm, _n_verts, p_vpl, &ti);
        return CLIP_NONE;
    } else {
        int temp_n;
        fix imax, imin, temp_i;
        grs_vertex *temp_p_vlist;

        temp_p_vlist = p_vlist;
        temp_n = n - 1;
        imax = imin = temp_p_vlist[temp_n].i;
        while (--n >= 0) {
            temp_i = temp_p_vlist[n].i;
            if (temp_i < imin)
                imin = temp_i;
            if (temp_i > imax)
                imax = temp_i;
        }

        temp_i = imax - imin;
        if (temp_i >= gr_clut_lit_tol) {
            ((void (*)(grs_bitmap * bm, int n, grs_vertex **vpl, grs_tmap_info *ti)) tmap_func)(bm, _n_verts, p_vpl,
                                                                                                &ti);
            return CLIP_NONE;
        } else {
            uchar *temp_ptr;

            imin += imax;
            imin >>= 9;
            imin &= 0xff00;
            temp_ptr = gr_get_light_tab() + imin;
            ti.clut = temp_ptr;
            ti.tmap_type += 2;
            ti.flags |= TMF_CLUT;

            ((void (*)(grs_bitmap * bm, int n, grs_vertex **vpl, grs_tmap_info *ti)) tmap_func)(bm, _n_verts, p_vpl,
                                                                                                &ti);
            return CLIP_NONE;
        }
    }
}

// return 1 if punt (ignore Z), 0 if use it
int check_linear(int n) {
    g3s_phandle temphand;
    fix zmin, zmax;
    fix temp;
    g3s_phandle *temp_vbuf2;

    temp_vbuf2 = vbuf2;
    n--;
    temphand = temp_vbuf2[n];
    zmax = zmin = temphand->gZ;

    while (--n >= 0) {
        temphand = temp_vbuf2[n];
        temp = temphand->gZ;

        if (temp < zmin)
            zmin = temp;
        if (temp > zmax)
            zmax = temp;
    }

    zmax -= zmin;
    temp = zmin >> flat8_per_ltol;
    if (temp >= zmax) {
        ti.tmap_type = GRC_BILIN + (light_flag << 1);
        tmap_func = (void *)&h_umap;
        return 1; // punt
    } else
        return 0; // use Z
}
