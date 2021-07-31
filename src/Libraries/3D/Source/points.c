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
// $Source: r:/prj/lib/src/3d/RCS/points.asm $
// $Revision: 1.17 $
// $Author: jaemz $
// $Date: 1994/09/28 19:00:52 $
//
// Point definition routines
//

#include "3d.h"
#include "3d_bitmap.h"
#include "globalv.h"
#include "lg.h"

// prototypes
void rotate_norm(g3s_vector *v, fix *x, fix *y, fix *z);
void do_norm_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz);

void do_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz);

// void xlate_rotate_point(g3s_vector *v, fix *x, fix *y, fix *z);
#define xlate_rotate_point(v, x, y, z) \
    do_rotate(v->gX - view_position.gX, v->gY - view_position.gY, v->gZ - view_position.gZ, x, y, z)

// for temp use in rotate_list, etc.
g3s_codes g_codes;

g3s_phandle g3_rotate_point(g3s_vector *v) {
    g3s_point *point;

    getpnt(point);
    xlate_rotate_point(v, &point->gX, &point->gY, &point->gZ);
    point->p3_flags = 0;

    code_point(point);
    return (point);
}

// matrix multiply and project a point. esi=vector, returns edi=point
g3s_phandle g3_transform_point(g3s_vector *v) {
    g3s_phandle tempH = g3_rotate_point(v);
    g3_project_point(tempH);
    return (tempH);
}

// takes edi = ptr to point. projects, fills in sx,sy, sets flag.
// returns 0 if z<=0, 1 if z>0.
// trashes eax,ecx,edx.
int g3_project_point(g3s_phandle p) {
    // check if this point is in front of the back plane.
    fix z = p->gZ;
    if (z <= 0)
        return 0;
    fix x = p->gX;
    fix y = p->gY;

    // point is in front of back plane---do projection.
    // project y coordinate.
    fix res = fix_mul_div(y, scrh, z);
    if (gOVResult) {
        p->codes |= CC_CLIP_OVERFLOW;
        return 1;
    }
    res = -res;
    if (AddLongWithOverflow(&res, res, biasy)) {
        p->codes |= CC_CLIP_OVERFLOW;
        return 1;
    }
    p->sy = res;

    // now project x point
    res = fix_mul_div(x, scrw, z);
    if (gOVResult) {
        p->codes |= CC_CLIP_OVERFLOW;
        return 1;
    }
    if (AddLongWithOverflow(&res, res, biasx)) {
        p->codes |= CC_CLIP_OVERFLOW;
        return 1;
    }
    p->sx = res;

    // modify point flags to indicate projection.
    p->p3_flags |= PF_PROJECTED;

    // point has been projected.
    return 1;
}

// takes esi=ptr to array of vectors, edi=ptr to list for point handles,
// ecx=count
g3s_codes g3_transform_list(short n, g3s_phandle *dest_list, g3s_vector *v) {
    g3s_phandle temphand;

    g_codes.or_ = 0;
    g_codes.and_ = 0xff;

    for (int i = n; i > 0; i--) {
        temphand = g3_transform_point(v++);
        g_codes.or_ |= temphand->codes;
        g_codes.and_ &= temphand->codes;

        *(dest_list++) = temphand;
    }
    return (g_codes);
}

// takes esi=ptr to normal vector. returns in <ecx,esi,eax>. trashes all regs
void rotate_norm(g3s_vector *v, fix *x, fix *y, fix *z) { do_norm_rotate(v->gX, v->gY, v->gZ, x, y, z); }

// does the rotate with the view matrix.
// takes <x,y,z> = <esi,edi,ebp>, returns <x,y,z> = <ecx,esi,eax>
void do_norm_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz) {
    int64_t r;

    // this matrix multiply here will someday be optimized for zero and one terms
    // uses unscaled rotation matrix.

    // first column
    r = fix64_mul(x, uvm1) + fix64_mul(y, uvm4) + fix64_mul(z, uvm7);
    *rx = fix64_to_fix(r);

    // second column
    r = fix64_mul(x, uvm2) + fix64_mul(y, uvm5) + fix64_mul(z, uvm8);
    *ry = fix64_to_fix(r);

    // third column
    r = fix64_mul(x, uvm3) + fix64_mul(y, uvm6) + fix64_mul(z, uvm9);
    *rz = fix64_to_fix(r);
}

// made this a define - MLA
/*//takes esi=ptr to vector. returns <x,y,z> in <ecx,esi,eax>. trashes all regs
void xlate_rotate_point(g3s_vector *v, fix *x, fix *y, fix *z)
 {
        do_rotate(v->gX-_view_position.gX, v->gY-_view_position.gY,
v->gZ-_view_position.gZ,x,y,z);
 }*/

// does the rotate with the view matrix.
// takes <x,y,z> = <esi,edi,ebp>, returns <x,y,z> = <ecx,esi,eax>
void do_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz) {
    // this matrix multiply here will someday be optimized for zero and one terms
    int64_t r;
    // first column
    r = fix64_mul(x, vm1) + fix64_mul(y, vm4) + fix64_mul(z, vm7);
    *rx = fix64_to_fix(r);

    // second column
    r = fix64_mul(x, vm2) + fix64_mul(y, vm5) + fix64_mul(z, vm8);
    *ry = fix64_to_fix(r);

    // third column
    r = fix64_mul(x, vm3) + fix64_mul(y, vm6) + fix64_mul(z, vm9);
    *rz = fix64_to_fix(r);
}

// add an x delta to a point. takes edi=point, eax=dx
// trashes eax,ebx,edx
void g3_add_delta_x(g3s_phandle p, fix dx) {
    p->gX += fix_mul(vm1, dx);
    p->gY += fix_mul(vm2, dx);
    p->gZ += fix_mul(vm3, dx);
    p->p3_flags &= ~PF_PROJECTED;

    code_point(p);
}

// add a z delta to a point. takes edi=point, eax=dz
// trashes eax,ebx,edx
void g3_add_delta_z(g3s_phandle p, fix dz) {
    p->gX += fix_mul(vm7, dz);
    p->gY += fix_mul(vm8, dz);
    p->gZ += fix_mul(vm9, dz);
    p->p3_flags &= ~PF_PROJECTED;

    code_point(p);
}

// like add_delta, but creates and returns a new point in edi
// add an x delta to a point. takes esi=point, eax=dx
// trashes eax,ebx,edx
g3s_phandle g3_copy_add_delta_x(g3s_phandle src, fix dx) {
    g3s_point *point;

    getpnt(point);
    point->gX = src->gX + fix_mul(dx, vm1);
    point->gY = src->gY + fix_mul(dx, vm2);
    point->gZ = src->gZ + fix_mul(dx, vm3);
    point->p3_flags = 0;
    code_point(point);
    return (point);
}

// like add_delta, but creates and returns a new point in edi
// add a y delta to a point. takes esi=point, eax=dy
// trashes eax,ebx,edx
g3s_phandle g3_copy_add_delta_y(g3s_phandle src, fix dy) {
    g3s_point *point;

    getpnt(point);
    point->gX = src->gX + fix_mul(dy, vm4);
    point->gY = src->gY + fix_mul(dy, vm5);
    point->gZ = src->gZ + fix_mul(dy, vm6);
    point->p3_flags = 0;
    code_point(point);
    return (point);
}

// like add_delta, but creates and returns a new point in edi
// add a z delta to a point. takes esi=point, eax=dz
// trashes eax,ebx,edx
g3s_phandle g3_copy_add_delta_z(g3s_phandle src, fix dz) {
    g3s_point *point;

    getpnt(point);
    point->gX = src->gX + fix_mul(dz, vm7);
    point->gY = src->gY + fix_mul(dz, vm8);
    point->gZ = src->gZ + fix_mul(dz, vm9);
    point->p3_flags = 0;
    code_point(point);
    return (point);
}

// like add_delta, but modifies an existing point in edi
// add an x delta to a point. takes esi=src point, edi=replace point, ax=dx
// trashes eax,ebx,edx
g3s_phandle g3_replace_add_delta_x(g3s_phandle src, g3s_phandle dst, fix dx) {
    dst->gX = src->gX + fix_mul(dx, vm1);
    dst->gY = src->gY + fix_mul(dx, vm2);
    dst->gZ = src->gZ + fix_mul(dx, vm3);
    dst->p3_flags = 0;
    code_point(dst);
    return (dst);
}

// like add_delta, but modifies an existing point in edi
// add a y delta to a point. takes esi=src point, edi=replace point, ax=dy
// trashes eax,ebx,edx
g3s_phandle g3_replace_add_delta_y(g3s_phandle src, g3s_phandle dst, fix dy) {
    dst->gX = src->gX + fix_mul(dy, vm4);
    dst->gY = src->gY + fix_mul(dy, vm5);
    dst->gZ = src->gZ + fix_mul(dy, vm6);
    dst->p3_flags = 0;
    code_point(dst);
    return (dst);
}

// like add_delta, but modifies an existing point in edi
// add a z delta to a point. takes esi=src point, edi=replace point, ax=dz
// trashes eax,ebx,edx
g3s_phandle g3_replace_add_delta_z(g3s_phandle src, g3s_phandle dst, fix dz) {
    dst->gX = src->gX + fix_mul(dz, vm7);
    dst->gY = src->gY + fix_mul(dz, vm8);
    dst->gZ = src->gZ + fix_mul(dz, vm9);
    dst->p3_flags = 0;
    code_point(dst);
    return (dst);
}

// like add_delta, but creates and returns a new point in edi
// add an xy delta to a point. takes edi=point, eax=dx, ebx=dy
// trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_xy(g3s_phandle src, fix dx, fix dy) {
    g3s_point *point;
    int64_t r;

    getpnt(point);

    // first column
    r = fix64_mul(dx, vm1) + fix64_mul(dy, vm4);
    point->gX = src->gX + fix64_to_fix(r);

    // second column
    r = fix64_mul(dx, vm2) + fix64_mul(dy, vm5);
    point->gY = src->gY + fix64_to_fix(r);

    // third column
    r = fix64_mul(dx, vm3) + fix64_mul(dy, vm6);
    point->gZ = src->gZ + fix64_to_fix(r);

    point->p3_flags = 0;
    code_point(point);
    return (point);
}

// like add_delta, but creates and returns a new point in edi
// add an xz delta to a point. takes edi=point, eax=dx, ebx=dz
// trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_xz(g3s_phandle src, fix dx, fix dz) {
    g3s_point *point;
    int64_t r;

    getpnt(point);

    // first column
    r = fix64_mul(dx, vm1) + fix64_mul(dz, vm7);
    point->gX = src->gX + fix64_to_fix(r);

    // second column
    r = fix64_mul(dx, vm2) + fix64_mul(dz, vm8);
    point->gY = src->gY + fix64_to_fix(r);

    // third column
    r = fix64_mul(dx, vm3) + fix64_mul(dz, vm9);
    point->gZ = src->gZ + fix64_to_fix(r);

    point->p3_flags = 0;
    code_point(point);
    return (point);
}

// like add_delta, but creates and returns a new point in edi
// add an yz delta to a point. takes edi=point, eax=dy, ebx=dz
// trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_yz(g3s_phandle src, fix dy, fix dz) {
    g3s_point *point;
    int64_t r;

    getpnt(point);

    // first column
    r = fix64_mul(dy, vm4) + fix64_mul(dz, vm7);
    point->gX = src->gX + fix64_to_fix(r);

    // second column
    r = fix64_mul(dy, vm5) + fix64_mul(dz, vm8);
    point->gY = src->gY + fix64_to_fix(r);

    // third column
    r = fix64_mul(dy, vm6) + fix64_mul(dz, vm9);
    point->gZ = src->gZ + fix64_to_fix(r);

    point->p3_flags = 0;
    code_point(point);
    return (point);
}
