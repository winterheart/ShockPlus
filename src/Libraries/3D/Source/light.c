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
// $Source: r:/prj/lib/src/3d/RCS/light.asm $
// $Revision: 1.2 $
// $Author: jaemz $
// $Date: 1994/10/13 20:51:49 $
//
// Light routines
//

#include "3d.h"
#include "globalv.h"
#include "lg.h"
#include "light.h"


// look ma, a zero vector
g3s_phandle tmp1;
g3s_phandle tmp2;
g3s_vector zero_vec = {0, 0, 0};

// should be normalized already
// multiply by _g3d_diff_light
// to set light intensity
void scale_light_vec(void) {
    g3d_light_vec.gX = fix_mul(g3d_light_vec.gX, g3d_diff_light);
    g3d_light_vec.gY = fix_mul(g3d_light_vec.gY, g3d_diff_light);
    g3d_light_vec.gZ = fix_mul(g3d_light_vec.gZ, g3d_diff_light);
}

// evaluates light point relative to another point, src is in world coords
// and pos is already transformed into eye coords
// void g3_eval_loc_light(eax)//
void g3_eval_loc_light(g3s_phandle pos) {
    fix temp;

    // transform light src point to eye coords

    // take difference with pos, and unscale them
    temp = -(pos->gX - g3d_light_trans.gX);
    g3d_light_vec.gX = fix_div(temp, matrix_scale.gX);

    temp = -(pos->gY - g3d_light_trans.gY);
    g3d_light_vec.gY = fix_div(temp, matrix_scale.gY);

    temp = -(pos->gZ - g3d_light_trans.gZ);
    g3d_light_vec.gZ = fix_div(temp, matrix_scale.gZ);

    // normalize vector
    g3_vec_normalize(&g3d_light_vec);

    // multiply by diff and divide by scale
    // so dot product just works
    scale_light_vec();
}

// evaluate the view vector relative to a point
// similar to above except src is always 0,0,0
// pos in eax has to be transformed into viewer coords
// eax points to the point in 3d space
void g3_eval_view(g3s_phandle pos) {
    fix temp;

    g3d_view_vec.gX = pos->gX - view_position.gX;
    g3d_view_vec.gY = pos->gY - view_position.gY;
    g3d_view_vec.gZ = pos->gZ - view_position.gZ;

    // normalize
    g3_vec_normalize(&g3d_view_vec);

    // multiply by spec and negate vector
    // since it currently points at the
    // point instead of the viewer
    // you might think, why not do this
    // afterwards, and you're right.  But only
    // only if this view vec only gets used once

    temp = -g3d_spec_light;
    g3d_view_vec.gX = fix_mul(g3d_view_vec.gX, temp);
    g3d_view_vec.gY = fix_mul(g3d_view_vec.gY, temp);
    g3d_view_vec.gZ = fix_mul(g3d_view_vec.gZ, temp);
}

// takes the dot product of view and light, for specular light
// assumes both light_vec and view_vec have been evaluated already
// everything is normal and in object space
// void g3_eval_ldotv(void)
void g3_eval_ldotv(void) {
    // multiply and scale once to find true dotproduct.
    // I hate this scaling stuff.  Erg.
    // transforming the light and view vectors into object
    // space would avoid this entirely

    g3d_ldotv = g3_vec_dotprod(&g3d_light_vec, &g3d_view_vec);
}

// check to see if local stuff has to get set and
// set it if necessary
// takes args in tmp1,tmp2
void check_for_near(void) {
    if (!(g3d_light_type & (LT_NEAR_VIEW | LT_NEAR_LIGHT)))
        return;

    // if light near, evaluate
    if (g3d_light_type & LT_NEAR_LIGHT)
        g3_eval_loc_light(tmp2);

    // if view near, eval
    if (g3d_light_type & LT_NEAR_VIEW)
        g3_eval_view(tmp2);

    // evaluate ldotv if either was local
    // MLA - this is stupid, the code that tests for whether or not to call
    // g3_eval_ldotv makes no sense, it does a JZ on an undetermined condition
    // code setup.  So I just call it all the time. Look in Light.ASM in the PC 3D
    // code for the original stuff.
    g3_eval_ldotv();
}

// raw version
// dot product with normal
// esi and edi
// ret eax
fix light_diff_raw(g3s_phandle src, g3s_phandle dest) {
    fix temp = g3_vec_dotprod(&g3d_light_vec, (g3s_vector *)src);

    // set lighting value in norm
    // test eax for negativity, zero if negative
    if (temp < 0)
        temp = 0;
    temp += g3d_amb_light; // add ambient light
    temp >>= 4;             // convert to fix16, consider row 16 normal
    dest->i = temp;
    return (temp);
}

// pure specular lighting is equal to
// 2(s.l)(s.v) - (l.v)
// take (s.l)
fix light_spec_raw(g3s_phandle src, g3s_phandle dest) {
    fix temp = g3_vec_dotprod(&g3d_light_vec, (g3s_vector *)src);
    if (temp < 0) {
        dest->i = g3d_amb_light >> 4;
        return (dest->i);
    }

    g3d_sdotl = temp;

    // take (s.v), note that this is jnorm, if its been done
    // we can eliminate this step intelligently somehow
    g3d_sdotv = temp = g3_vec_dotprod(&g3d_view_vec, (g3s_vector *)tmp1);
    temp <<= 1;                       // multiply (s.v) by 2
    temp = fix_mul(temp, g3d_sdotl); // mult by (s.l)
    temp -= g3d_ldotv;               // subtract ldotv, done!

    // test eax for flash point zero if under
    // or better test eax for spec threshhold
    if (temp < g3d_flash)
        temp = 0;

    // add ambient light
    temp += g3d_amb_light;

    // check to see if its greater than the max row
    // and truncate if it is
    if (temp >= (LT_TABSIZE << 12))
        ;
    temp = (LT_TABSIZE << 12) - 1; // if its over the max, set it to just under max

    dest->i = temp >> 4; // convert to fix16, consider row 16 normal
    return (temp);
}

// raw version of dands without local checking
fix light_dands_raw(g3s_phandle src, g3s_phandle dest) {
    fix temp;

    // pure specular lighting is equal to
    // 2(s.l)(s.v) - (l.v)
    // take (s.l) if neg, you know you're done, surface HAS to face the light

    temp = g3_vec_dotprod(&g3d_light_vec, (g3s_vector *)src);
    if (temp < 0) {
        dest->i = g3d_amb_light >> 4;
        return (dest->i);
    }
    g3d_sdotl = temp;

    // take (s.v), note that this is jnorm, if its been done
    // we can eliminate this step intelligently somehow
    g3d_sdotv = temp = g3_vec_dotprod(&g3d_view_vec, (g3s_vector *)tmp1);
    temp = fix_mul(temp, g3d_sdotl); // mult by (s.l)
    temp <<= 1;                       // multiply (s.v)(s.l) by 2
    temp -= g3d_ldotv;               // subtract ldotv, done!

    // test eax for flash point zero if under
    // or better test eax for spec threshhold
    if (temp < g3d_flash)
        temp = 0;

    // add diffuse component & ambient light
    temp += g3d_sdotl + g3d_amb_light;

    // check to see if its greater than the max row
    // and truncate if it is
    if (temp >= (LT_TABSIZE << 12))
        temp = (LT_TABSIZE << 12) - 1; // if its over the max, set it to just under max

    dest->i = temp >> 4; // convert to fix16, consider row 16 normal
    return (temp);
}

// farms out a point based on flags
// void g3_light(g3s_phandle norm,g3s_phandle pos)//
//[eax,edx]
fix g3_light(g3s_phandle norm, g3s_phandle pos) {
    g3s_phandle temp;

    if ((g3d_light_type & LT_GOUR) == 0)
        temp = norm;
    else
        temp = pos;

    tmp1 = norm;
    tmp2 = pos;

    if ((g3d_light_type & (LT_NEAR_VIEW | LT_NEAR_LIGHT)) != 0)
        check_for_near();

    // determine which routine to jump to based on flags
    switch (g3d_light_type) {
    case LT_DIFF:
        return (light_diff_raw(tmp1, temp));
    case LT_SPEC:
        return (light_spec_raw(tmp1, temp));
    default:
        return (light_dands_raw(tmp1, temp));
    }
}

// farms out a point based on flags
// void g3_light_obj(g3s_vector *norm,g3s_vector *pos)//
//[eax,edx]
// norm is set with only 15 bits of fraction, pos is normal
// all vectors are in object space
// though we put these in points, they are in object space,
// not world space
void g3_light_obj(g3s_phandle norm, g3s_phandle pos) {
    g3s_point *norm_point;
    g3s_point *pos_point;
    fix shade;

    tmp1 = norm;
    tmp2 = pos;
    getpnt(norm_point);

    norm_point->gX = norm->gX << 1;
    norm_point->gY = norm->gY << 1;
    norm_point->gZ = norm->gZ << 1;

    // Copy position over to its own point
    getpnt(pos_point);
    *(g3s_vector *)pos_point = *(g3s_vector *)pos;

    shade = g3_light(norm_point, pos_point);

    // set the lighting when non gouraud
    // set fill type to address of shading table
    shade &= 0xffffff00;
    shade += g3d_light_tab;

    gr_set_fill_parm(shade);

    freepnt(norm_point);
    freepnt(pos_point);
}
