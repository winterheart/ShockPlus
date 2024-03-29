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
// $Source: n:/project/lib/src/3d/RCS/vector.asm $
// $Revision: 1.3 $
// $Author: dc $
// $Date: 1993/08/10 22:54:29 $
//
// Vector math routines for 3d library
//
// $Log: vector.asm $
// Revision 1.3  1993/08/10  22:54:29  dc
// add _3d.inc to includes
//
// Revision 1.2  1993/05/11  15:03:18  matt
// Changed g3_vec_scale() to take seperate dest & src
// Fixed bug in vector compute
//
// Revision 1.1  1993/05/04  17:39:56  matt
// Initial revision
//
//

//#include <FixMath.h>

#include <math.h> // sqrtl()

#include "3d.h"
#include "globalv.h"
#include "fix.h"
#include "lg.h"
#include "vector.h"

// adds two vectors:  edi = esi + ebx
void g3_vec_add(g3s_vector *dest, g3s_vector *src1, g3s_vector *src2) {
    dest->gX = src1->gX + src2->gX;
    dest->gY = src1->gY + src2->gY;
    dest->gZ = src1->gZ + src2->gZ;
}

// subtracts two vectors:  edi = esi - ebx. trashes eax
void g3_vec_sub(g3s_vector *dest, g3s_vector *src1, g3s_vector *src2) {
    dest->gX = src1->gX - src2->gX;
    dest->gY = src1->gY - src2->gY;
    dest->gZ = src1->gZ - src2->gZ;
}

// scale a vector in place. takes edi=dest vector, esi=src vector, ebx=scale
void g3_vec_scale(g3s_vector *dest, g3s_vector *src, fix s) {
    dest->gX = fix_mul(src->gX, s);
    dest->gY = fix_mul(src->gY, s);
    dest->gZ = fix_mul(src->gZ, s);
}

// fix mag(vector *v)
// takes esi = v. returns mag in eax. trashes all but ebp
fix g3_vec_mag(g3s_vector *v) {
    int64_t result = fix64_mul(v->gX, v->gX) + fix64_mul(v->gY, v->gY) + fix64_mul(v->gZ, v->gZ);
    return (fix)sqrtl(result);
}

// compute dot product of vectors at [esi] & [edi]
fix g3_vec_dotprod(g3s_vector *v0, g3s_vector *v1) {
    int64_t result = fix64_mul(v0->gX, v1->gX) + fix64_mul(v0->gY, v1->gY) + fix64_mul(v0->gZ, v1->gZ);

    return fix64_to_fix(result);
}

// normalizes the vector at esi. trashes all but esi,ebp
void g3_vec_normalize(g3s_vector *v) {
    fix temp;

    temp = g3_vec_mag(v);

    v->gX = fix_div(v->gX, temp);
    v->gY = fix_div(v->gY, temp);
    v->gZ = fix_div(v->gZ, temp);
}

// compute surface normal from three points. DOES NOT NORMALIZE!
// takes edi=dest, eax,edx,ebx = points. fills in [edi].
// trashes eax,ebx,ecx,edx,esi
// the quick version does not normalize
void g3_compute_normal_quick(g3s_vector *v, g3s_vector *v0, g3s_vector *v1, g3s_vector *v2) {
    int64_t r_temp, r[3];
    g3s_vector temp_v0;
    g3s_vector temp_v1;
    int32_t temp_long = 0;
    int32_t shiftcount;

    g3_vec_sub(&temp_v0, v1, v0);
    g3_vec_sub(&temp_v1, v2, v1);

    // dest->x = v1z * v0y - v1y * v0z;
    r[0] = fix64_mul(temp_v1.gZ, temp_v0.gY) - fix64_mul(temp_v1.gY, temp_v0.gZ);
    v->gX = fix64_frac(r[0]);

    // dest->y = v1x * v0z - v1z * v0x;
    r[1] = fix64_mul(temp_v1.gX, temp_v0.gZ) - fix64_mul(temp_v1.gZ, temp_v0.gX);
    v->gY = fix64_frac(r[1]);

    // dest->z = v1y * v0x - v1x * v0y;
    r[2] = fix64_mul(temp_v1.gY, temp_v0.gX) - fix64_mul(temp_v1.gX, temp_v0.gY);
    v->gZ = fix64_frac(r[2]);

    // see if fit into a longword
    for (int i = 0; i < 3; i++) {
        r_temp = r[i];
        if (r_temp < 0)
            r_temp = -r_temp;
        temp_long |= fix64_int(2 * r_temp);
    }
    if (!temp_long)
        return; // everything fits in the low longword. hurrah. see ya.

    // see how far to shift to fit in a longword
    shiftcount = 0;
    while (temp_long >= 0x0100) {
        shiftcount += 8;
        temp_long >>= 8;
    }
    shiftcount += shift_table[temp_long];

    // now get the results
    for (int i = 0; i < 3; i++) {
        r[i] >>= shiftcount;
        v->xyz[i] = fix64_frac(r[i]);
    }
}
