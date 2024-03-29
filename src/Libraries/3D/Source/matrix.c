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
// $Source: r:/prj/lib/src/3d/RCS/matrix.C $
// $Revision: 1.17 $
// $Author: jaemz $
// $Date: 1994/09/20 13:33:41 $
//
// Matrix setup and multiply routines
//
// $Log: matrix.asm $
// Revision 1.17  1994/09/20  13:33:41  jaemz
// *** empty log message ***
//
// Revision 1.16  1994/08/18  03:46:59  jaemz
// Changed stereo glob names to have underscore for c
//
// Revision 1.15  1994/08/04  16:36:13  jaemz
// *** empty log message ***
//
// Revision 1.14  1994/07/19  13:48:35  jaemz
// Added support for stereo
//
// Revision 1.13  1994/07/15  19:31:23  jaemz
// changed view_zoom to _view_zoom for c access
//
// Revision 1.12  1994/07/15  14:13:37  jaemz
// Added _view_position with an underscore to make it c readable
//
// Revision 1.11  1994/06/02  15:09:36  junochoe
// changed matrix_scale to _matrix_scale
//
// Revision 1.10  1994/02/08  20:46:39  kaboom
// Moved back clipping plane to z=1\65536.
//
// Revision 1.9  1993/12/14  14:04:23  kevin
// Swap and negate axis before saving unscaled view matrix.
// Also commented out code that breaks citadel under wvideo.
//
// Revision 1.8  1993/10/02  09:28:38  kaboom
// Changed point coder to check for magic minimum z value.
//
// Revision 1.7  1993/08/10  22:54:16  dc
// add _3d.inc to includes
//
// Revision 1.6  1993/07/13  11:58:05  kaboom
// Fixed bugs in saving off of angles.
//
// Revision 1.5  1993/07/08  23:39:01  kaboom
// Now sets pitch, heading, bank, values for scaler/roller.
//
// Revision 1.4  1993/06/22  18:35:35  kaboom
// Changed g3_matrix_x_matrix to g3_matrix_x_matrix_ so it's callable
// from watcom C w/register passing.
//
// Revision 1.3  1993/05/24  15:48:56  matt
// process_view_matrix now copies view_matrix to unscaled_matrix before
// messing with it.
//
// Revision 1.2  1993/05/11  15:03:58  matt
// Added g3_get_view_pyramid()
//
// Revision 1.1  1993/05/04  17:39:51  matt
// Initial revision
//
//

#include "3d.h"
#include "globalv.h"
#include "lg.h"
#include "matrix.h"

fix sinp;   // fix	?
fix cosp;   // fix	?
fix sinb;   // fix	?
fix cosb;   // fix	?
fix sinh_s; // fix	?
fix cosh_s; // fix	?

// vars for get_view_pyramid
fix d13; // fix	?
fix d23; // fix	?
fix d46; // fix	?
fix d56; // fix	?
fix d79; // fix	?
fix d89; // fix	?
fix den; // fix	?

// prototypes


void compute_XYZ(g3s_matrix *view_matrix);
void compute_YXZ(g3s_matrix *view_matrix);
void compute_YZX(g3s_matrix *view_matrix);
void compute_XZY(g3s_matrix *view_matrix);
void compute_ZXY(g3s_matrix *view_matrix);
void compute_ZYX(g3s_matrix *view_matrix);
void compute_invalid(g3s_matrix *view_matrix);

// function table
void (*rotation_table[])(g3s_matrix *) = {
    compute_XYZ,     //
    compute_YXZ,     //
    compute_invalid, // invalid
    compute_YZX,     // not implemented
    compute_XZY,     // not implemented
    compute_invalid, // invalid
    compute_ZXY,     // not implemented
    compute_ZYX      // not implemented
};

// build the view matrix from view angles, etc.
// takes esi=pos, ebx=angles, eax=zoom, ecx=rotation order
void g3_set_view_angles(g3s_vector *pos, g3s_angvec *angles, int rotation_order, fix zoom) {
    view_zoom = zoom; // mov	_view_zoom,eax	;save zoom
    view_position = *pos;

    view_pitch = angles->tx;
    view_heading = angles->ty;
    view_bank = angles->tz;

    angles_2_matrix(angles, &view_matrix, rotation_order);
    process_view_matrix();
}

// build the view matrix from an object matrix
// takes esi=pos, ebx=matrix, eax=zoom
void g3_set_view_matrix(g3s_vector *pos, g3s_matrix *m, fix zoom) {
    view_zoom = zoom;
    view_position = *pos;
    view_matrix = *m;

    process_view_matrix();
}

// generates a matrix from 3 angles. esi=angles,edi=matrix, ecx=order
// trashes esi, ebx, ecx, eax, edx, plus whatever fix_sincos trashes
// note that these routines use variables called sinp,sinb,etc., which
// are really just rotations around certain axes, and don't necessarily
// mean pitch,bank, or heading in each coordinated system
void angles_2_matrix(g3s_angvec *angles, g3s_matrix *view_matrix, int rotation_order) {
    fix_sincos(angles->tx, &sinp, &cosp);
    fix_sincos(angles->ty, &sinh_s, &cosh_s);
    fix_sincos(angles->tz, &sinb, &cosb);

    rotation_table[rotation_order](view_matrix);
}

// compute a matrix with the given order.  takes sin & cos vars set, edi=dest
void compute_XYZ(g3s_matrix *view_matrix) {
    fix cbsp, cpsb, cpshsb, spsb, cpshcb, spshcb, spshsb, cpcb;

    view_matrix->m1 = fix_mul(cosh_s, cosb);  // m1 = chcb
    view_matrix->m2 = -fix_mul(cosh_s, sinb); // m2 = -chsb
    view_matrix->m3 = sinh_s;                 // m3 = sinh

    cbsp = fix_mul(cosb, sinp);
    spshcb = fix_mul(cbsp, sinh_s);
    cpsb = fix_mul(cosp, sinb);
    view_matrix->m4 = cpsb + spshcb; // m4 = cpsb+spshcb

    cpshsb = fix_mul(cpsb, sinh_s);
    view_matrix->m8 = cbsp + cpshsb; // m8 = spcb+cpshsb

    spsb = fix_mul(sinp, sinb);
    spshsb = fix_mul(spsb, sinh_s);
    cpcb = fix_mul(cosp, cosb);
    view_matrix->m5 = cpcb - spshsb; // m5 = cpcb-spshsb

    cpshcb = fix_mul(cpcb, sinh_s);
    view_matrix->m7 = spsb - cpshcb; // m7 = spsb-cpshcb

    view_matrix->m6 = -fix_mul(sinp, cosh_s); // m6 = -spch
    view_matrix->m9 = fix_mul(cosp, cosh_s);  // m9 = cpch
}

void compute_YXZ(g3s_matrix *view_matrix) {
    fix cbch, sbsh, sbch, cbsh;

    // m1 = cb*ch + sb*sp*sh
    cbch = fix_mul(cosb, cosh_s);
    sbsh = fix_mul(sinb, sinh_s);
    view_matrix->m1 = cbch + fix_mul(sbsh, sinp);

    // m8 = sb*sh + cb*ch*sp
    view_matrix->m8 = sbsh + fix_mul(cbch, sinp);

    // m2 = -sb*ch + cb*sp*sh
    sbch = fix_mul(sinb, cosh_s);
    cbsh = fix_mul(cosb, sinh_s);
    view_matrix->m2 = fix_mul(cbsh, sinp) - sbch;

    // m7 = -cb*sh + sb*ch*sp
    view_matrix->m7 = fix_mul(sbch, sinp) - cbsh;

    // m3 = sh*cp
    view_matrix->m3 = fix_mul(sinh_s, cosp);

    // m4 = sb*cp
    view_matrix->m4 = fix_mul(sinb, cosp);

    // m5 = cb*cp
    view_matrix->m5 = fix_mul(cosb, cosp);

    // m6 = - sp
    view_matrix->m6 = -sinp;

    // m9 = ch*cp
    view_matrix->m9 = fix_mul(cosh_s, cosp);
}

void compute_YZX(g3s_matrix *view_matrix) { DEBUG("%s: needs to be implemented", __FUNCTION__); }

void compute_XZY(g3s_matrix *view_matrix) { DEBUG("%s: needs to be implemented", __FUNCTION__); }

void compute_ZXY(g3s_matrix *view_matrix) { DEBUG("%s: needs to be implemented", __FUNCTION__); }

void compute_ZYX(g3s_matrix *view_matrix) { DEBUG("%s: needs to be implemented", __FUNCTION__); }

// invalid does nothing (and does it well!)
void compute_invalid(g3s_matrix *view_matrix) {}

// scale, fix, etc, the view matrix
void process_view_matrix(void) {

    // adjust matrix for user's coordinate system
    if ((axis_swap_flag & 1) != 0) {
        SwapFix(vm1, vm2);
        SwapFix(vm4, vm5);
        SwapFix(vm7, vm8);
    }
    if ((axis_swap_flag & 2) != 0) {
        SwapFix(vm1, vm3);
        SwapFix(vm4, vm6);
        SwapFix(vm7, vm9);
    }
    if ((axis_swap_flag & 4) != 0) {
        SwapFix(vm2, vm3);
        SwapFix(vm5, vm6);
        SwapFix(vm8, vm9);
    }

    // get vars for horizon drawer
    horizon_vector.gX = ((fix *)&view_matrix)[up_axis];
    horizon_vector.gY = ((fix *)&view_matrix)[up_axis + 1];
    horizon_vector.gZ = ((fix *)&view_matrix)[up_axis + 2];

    // now fix signs
    if ((axis_neg_flag & 1) != 0) {
        vm1 = -vm1;
        vm4 = -vm4;
        vm7 = -vm7;
    }

    if ((axis_neg_flag & 2) != 0) {
        vm2 = -vm2;
        vm5 = -vm5;
        vm8 = -vm8;
    }

    if ((axis_neg_flag & 4) != 0) {
        vm3 = -vm3;
        vm6 = -vm6;
        vm9 = -vm9;
    }

    unscaled_matrix = view_matrix;
    scale_view_matrix();
}

// performs various scaling and other operations on the view matrix
void scale_view_matrix(void) {
    int32_t temp_long;
    fix temp_fix;

    // set matrix scale vector based on zoom
    matrix_scale.gX = FIX_UNIT; // use 1.0 as defaults
    matrix_scale.gY = FIX_UNIT;
    matrix_scale.gZ = FIX_UNIT;

    if (view_zoom <= FIX_UNIT)
        matrix_scale.gZ = view_zoom;
    else
        matrix_scale.gY = matrix_scale.gX = fix_div(FIX_UNIT, view_zoom);

    // scale set matrix scale vector based on window and pixel ratio
    temp_long = fix_mul_div(window_height, pixel_ratio, window_width);

    if (temp_long <= FIX_UNIT)
        matrix_scale.gX = fix_mul(matrix_scale.gX, temp_long);
    else
        matrix_scale.gY = fix_div(matrix_scale.gY, temp_long);

    // now actually scale the matrix
    temp_fix = matrix_scale.gX;
    vm1 = fix_mul(vm1, temp_fix);
    vm4 = fix_mul(vm4, temp_fix);
    vm7 = fix_mul(vm7, temp_fix);

    temp_fix = matrix_scale.gY;
    vm2 = fix_mul(vm2, temp_fix);
    vm5 = fix_mul(vm5, temp_fix);
    vm8 = fix_mul(vm8, temp_fix);

    temp_fix = matrix_scale.gZ;
    vm3 = fix_mul(vm3, temp_fix);
    vm6 = fix_mul(vm6, temp_fix);
    vm9 = fix_mul(vm9, temp_fix);

    // scale horizon vector
    horizon_vector.gX = fix_mul(fix_mul(matrix_scale.gY, matrix_scale.gZ), horizon_vector.gX);
    horizon_vector.gY = fix_mul(fix_mul(matrix_scale.gX, matrix_scale.gZ), horizon_vector.gY);
    horizon_vector.gZ = fix_mul(fix_mul(matrix_scale.gX, matrix_scale.gY), horizon_vector.gZ);
}

// takes point in edi, set codes in point, returns codes in bl
// trashes eax,bl
// note: in an effort to optimize the coder, I tried several variants,
// including the C&D coder, and a clever one using the set<cond> instruction
// that contained no jumps.  On my (Matt's) 486, this dull, straightforward
// one was just as fast as any other, and short, too.
int code_point(g3s_point *pt) {
    int code;
    int tempX, tempY, tempZ;

    tempX = pt->gX;
    tempY = pt->gY;
    tempZ = pt->gZ;
    code = 0;
    if (tempX > tempZ)
        code |= CC_OFF_RIGHT;
    if (tempY > tempZ)
        code |= CC_OFF_TOP;

    tempZ = -tempZ;
    if (tempZ > -1)
        code |= CC_BEHIND;

    if (tempX < tempZ)
        code |= CC_OFF_LEFT;
    if (tempY < tempZ)
        code |= CC_OFF_BOT;

    pt->codes = code;
    return (code);
}

// general matrix multiply. takes esi=src vector, edi=matrix, ebx=dest vector
// src and dest vectors can be the same
void g3_vec_rotate(g3s_vector *dest, g3s_vector *src, g3s_matrix *m) {
    int32_t srcX, srcY, srcZ; // in locals for PPC speed
    int64_t r;

    srcX = src->gX;
    srcY = src->gY;
    srcZ = src->gZ;

    // first column
    r = fix64_mul(srcX, m->m1) + fix64_mul(srcY, m->m4) + fix64_mul(srcZ, m->m7);
    dest->gX = fix64_to_fix(r);

    // second column
    r = fix64_mul(srcX, m->m2) + fix64_mul(srcY, m->m5) + fix64_mul(srcZ, m->m8);
    dest->gY = fix64_to_fix(r);

    // third column
    r = fix64_mul(srcX, m->m3) + fix64_mul(srcY, m->m6) + fix64_mul(srcZ, m->m9);
    dest->gZ = fix64_to_fix(r);
}

// transpose a matrix at esi in place trashes eax
void g3_transpose(g3s_matrix *m) // transpose in place
{
    SwapFix(m->m2, m->m4);
    SwapFix(m->m3, m->m7);
    SwapFix(m->m6, m->m8);
}

// MLA- oh no I've got LookingGlass disease, I'm making multi-line #defines!
// No worries, WH comes to help!
fix mxm_mul(fix s1_1, fix s1_2, fix s1_3, fix s2_1, fix s2_2, fix s2_3) {
    int64_t r = fix64_mul(s1_1, s2_1) + fix64_mul(s1_2, s2_2) + fix64_mul(s1_3, s2_3);
    return fix64_to_fix(r);
}

// matrix by matrix multiply:  ebx = esi * edi
// does ebx = edi * esi
// it does the inverse actually, edi*esi, assuming
//  standard layout:
//  147
//  258
//  369
//
// dest = bx, src1 = si, src2 = di
void g3_matrix_x_matrix(g3s_matrix *dest, g3s_matrix *src1, g3s_matrix *src2) {
    dest->m1 = mxm_mul(src1->m1, src1->m2, src1->m3, src2->m1, src2->m4, src2->m7);
    dest->m2 = mxm_mul(src1->m1, src1->m2, src1->m3, src2->m2, src2->m5, src2->m8);
    dest->m3 = mxm_mul(src1->m1, src1->m2, src1->m3, src2->m3, src2->m6, src2->m9);

    dest->m4 = mxm_mul(src1->m4, src1->m5, src1->m6, src2->m1, src2->m4, src2->m7);
    dest->m5 = mxm_mul(src1->m4, src1->m5, src1->m6, src2->m2, src2->m5, src2->m8);
    dest->m6 = mxm_mul(src1->m4, src1->m5, src1->m6, src2->m3, src2->m6, src2->m9);

    dest->m7 = mxm_mul(src1->m7, src1->m8, src1->m9, src2->m1, src2->m4, src2->m7);
    dest->m8 = mxm_mul(src1->m7, src1->m8, src1->m9, src2->m2, src2->m5, src2->m8);
    dest->m9 = mxm_mul(src1->m7, src1->m8, src1->m9, src2->m3, src2->m6, src2->m9);
}

static int64_t cross(int v1, int v2, int v3, int v4) { return (int64_t)v1 * v2 - (int64_t)v3 * v4; }

// fills in edi with vector. takes deltas set
void get_pyr_vector(g3s_vector *corners) {
    int64_t r;

    // calculate denominators, divide each by the longest of the three
    int64_t den_x = cross(d46, d89, d56, d79);
    int64_t den_y = cross(d23, d79, d13, d89);
    int64_t den_z = cross(d13, d56, d23, d46);

    if (llabs(den_x) >= llabs(den_y) && llabs(den_x) >= llabs(den_z)) {
        corners->gX = FIX_UNIT;
        corners->gY = den_y / (den_x >> 16);
        corners->gZ = den_z / (den_x >> 16);
    } else if (llabs(den_y) >= llabs(den_x) && llabs(den_y) >= llabs(den_z)) {
        corners->gX = den_x / (den_y >> 16);
        corners->gY = FIX_UNIT;
        corners->gZ = den_z / (den_y >> 16);
    } else {
        corners->gX = den_x / (den_z >> 16);
        corners->gY = den_y / (den_z >> 16);
        corners->gZ = FIX_UNIT;
    }

    // got_vector
    g3_vec_normalize(corners);

    // make sure vector points right way
    r = fix64_mul(corners->gX, vm3) + fix64_mul(corners->gY, vm6) + fix64_mul(corners->gZ, vm9);
    if (fix64_int(r) < 0) {
        corners->gX = -corners->gX;
        corners->gY = -corners->gY;
        corners->gZ = -corners->gZ;
    }
}

// fills in four vectors which are the corners of the view pyramid
// takes edi=ptr to array of 4 vectors. trashes all but edp
void g3_get_view_pyramid(g3s_vector *corners) {
    fix save_d23, save_d56, save_d89;

    d13 = vm1 - vm3;
    d23 = vm2 - vm3;
    d46 = vm4 - vm6;
    d56 = vm5 - vm6;
    d79 = vm7 - vm9;
    d89 = vm8 - vm9;
    get_pyr_vector(corners);
    corners++;

    save_d23 = d23;
    save_d56 = d56;
    save_d89 = d89;

    d23 -= vm2 << 1;
    d56 -= vm5 << 1;
    d89 -= vm8 << 1;
    get_pyr_vector(corners);
    corners++;

    d13 -= vm1 << 1;
    d46 -= vm4 << 1;
    d79 -= vm7 << 1;
    get_pyr_vector(corners);
    corners++;

    d23 = save_d23;
    d56 = save_d56;
    d89 = save_d89;
    get_pyr_vector(corners);
}
