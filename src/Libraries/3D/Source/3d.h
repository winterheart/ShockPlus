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
 * $Source: r:/prj/lib/src/3d/RCS/3d.h $
 * $Revision: 1.35 $
 * $Author: jaemz $
 * $Date: 1994/09/20 13:21:45 $
 *
 * Header file for LookingGlass 3D library
 *
 * $Log: 3d.h $

 *------------------------------------------------------*
 * MAC VERSION NOTES
 *
 *  All of the #pragma aux statements have been removed, because they only
 *  specify the register order parameters are passed into routines.  Since
 *  all this stuff has to work on both 68K and PowerPC machines, we have
 *  to use C calling conventions almost everwhere.
 *
 *------------------------------------------------------*

 * Revision 1.35  1994/09/20  13:21:45  jaemz
 * Added lighting features, took out old call interpret_object
 *
 * Revision 1.33  1994/08/28  14:48:05  kevin
 * Added linear mapping alternative for detail level stuff.
 *
 * Revision 1.32  1994/08/18  03:45:08  jaemz
 * Added call for g3_object_scale
 *
 * Revision 1.31  1994/07/21  00:28:25  jaemz
 * Added stereo declares
 *
 * Revision 1.29  1994/06/08  21:20:12  jaemz
 * Commented transform matrix
 *
 * Revision 1.28  1994/06/01  15:57:22  jaemz
 * Added a "*" to g3_compute_normal
 *
 * Revision 1.27  1994/05/31  16:38:36  jaemz
 * Added documenting comment to .h file about matrices being
 * column based
 *
 * Revision 1.26  1994/05/19  09:47:42  kevin
 * g3_light(draw)_t(l,floor_,wall_)map now use watcom register passing
 conventions.
 *
 * Revision 1.25  1994/05/02  23:39:01  kevin
 * Added prototypes for wall and floor map procedures.
 *
 * Revision 1.24  1994/02/08  20:48:06  kaboom
 * Added translucent polygon prototypes\pragmas.
 *
 * Revision 1.23  1993/12/15  01:52:17  dc
 * alloc_list
 *
 * Revision 1.22  1993/12/14  22:58:04  kevin
 * Added declarations for biasx,biasy, and perspective mapper context
 manipulation routines.
 *
 * Revision 1.21  1993/12/11  04:03:12  kevin
 * Added declarations for g3_rotate_grad and _norm.
 *
 * Revision 1.20  1993/12/06  15:51:25  unknown
 * c:\app\star24\vmode 132x44 uv mappers to take a phandle *, as they do
 *
 * Revision 1.19  1993/12/04  17:00:02  kaboom
 * Added declarations for bitmap lighters.
 *
 * Revision 1.18  1993/11/07  09:02:15  dc
 * support for replace_add_delta along axis
 *
 * Revision 1.17  1993/10/22  09:37:11  kaboom
 * Added new prototypes and pragmas for linear map routines.
 *
 * Revision 1.16  1993/10/02  11:01:36  kaboom
 * Changed names of clip_{line,polygon} to g3_clip_{line,polygon} to avoid
 * name collisions.
 *
 * Revision 1.15  1993/10/02  09:11:30  kaboom
 * Added declarations for scrw,scrh.  Added prototypes&pragmas for new
 * clipping routines and g3_{draw,light}_tmap.  New inline for g3_set_vcolor.
 *
 * Revision 1.14  1993/08/11  15:02:42  kaboom
 * Added prototypes and pragmas for lighting texture mappers.
 *
 * Revision 1.13  1993/08/04  00:48:39  dc
 * move interpreter zaniness to separate file
 *
 * Revision 1.12  1993/07/08  23:36:08  kaboom
 * Added prototypes for g3_bitmap(), g3_anchor_bitmap(), and
 * g3_set_bitmap_scale().
 *
 * Revision 1.11  1993/06/30  11:20:01  spaz
 * Added prototypes for g3_draw_cline, g3_draw_sline
 *
 * Revision 1.10  1993/06/18  15:47:24  kaboom
 * Updated prototype for g3_project_point to reflect return value.
 *
 * Revision 1.9  1993/06/09  04:23:46  kaboom
 * Changed prototype and comment for g3_draw_tmap_tile to reflect new usage.
 *
 * Revision 1.8  1993/06/04  16:53:29  matt
 * Added hacks to get around c++ non-support of anonymous structures in unions.
 *
 * Revision 1.7  1993/05/24  15:51:04  matt
 * Added g3_get_slew_step(), removed g3_draw_smooth_horizon(), changed a few
 * comments.
 *
 * Revision 1.6  1993/05/21  16:09:17  matt
 * Added new way to specifiy axis orientation, which may be more intuitive
 *
 * Revision 1.5  1993/05/13  12:17:08  matt
 * Added new function, g3_check_codes()
 *
 * Revision 1.4  1993/05/11  15:23:49  matt
 * Changed comment for g3_draw_tmap_tile() to reflect new functionality
 *
 * Revision 1.3  1993/05/11  14:57:53  matt
 * Changed g3_vec_scale() to takes seperate dest & src.
 * Added g3_get_view_pyramid().
 *
 * Revision 1.2  1993/05/10  13:25:14  matt
 * Added the ability to access the elements of a vector as an array. Fields
 * x,y,z can be accessed as xyz[0..2].
 *
 * Revision 1.1  1993/05/04  17:39:56  matt
 * Initial revision
 */

#ifndef __3D_H
#define __3D_H

#include <stdbool.h>

#include "2d.h"
#include "fix.h"

#if defined(__cplusplus)
extern "C" {
#endif // !defined(__cplusplus)

#pragma pack(push, 2)

// MLA defines
#define SwapFix(x, y)   \
    {                   \
        fix temp = (x); \
        (x) = (y);      \
        (y) = temp;     \
    }

#define vm1 view_matrix.m1
#define vm2 view_matrix.m2
#define vm3 view_matrix.m3
#define vm4 view_matrix.m4
#define vm5 view_matrix.m5
#define vm6 view_matrix.m6
#define vm7 view_matrix.m7
#define vm8 view_matrix.m8
#define vm9 view_matrix.m9

#define uvm1 unscaled_matrix.m1
#define uvm2 unscaled_matrix.m2
#define uvm3 unscaled_matrix.m3
#define uvm4 unscaled_matrix.m4
#define uvm5 unscaled_matrix.m5
#define uvm6 unscaled_matrix.m6
#define uvm7 unscaled_matrix.m7
#define uvm8 unscaled_matrix.m8
#define uvm9 unscaled_matrix.m9

// gets the next available pnt in reg.
#define getpnt(res)               \
    {                             \
        g3s_point *scratch;       \
        if ((res = first_free)) { \
            scratch = res->next;  \
            first_free = scratch; \
        }                         \
    }

// frees the point in the specified register. uses ebx as scratch
#define freepnt(src)                     \
    {                                    \
        g3s_point *scratch = first_free; \
        src->next = scratch;             \
        first_free = src;                \
    }

// constants

// these are for rotation order when generating a matrix from angles
// bit 2 means y before z if 0, 1 means x before z if 0, 0 means x before y if 0

#define ORDER_XYZ 0 // 000
#define ORDER_YXZ 1 // 001
#define ORDER_YZX 3 // 011
#define ORDER_XZY 4 // 100
#define ORDER_ZXY 6 // 110
#define ORDER_ZYX 7 // 111

// To specify user's coordinate system: use one of these for each user_x_axis,
// user_y_axis,& user_z_axis in g3_init to tell the 3d what your x,y,& z mean.

#define AXIS_RIGHT 1
#define AXIS_UP 2
#define AXIS_IN 3
#define AXIS_LEFT (-AXIS_RIGHT)
#define AXIS_DOWN (-AXIS_UP)
#define AXIS_OUT (-AXIS_IN)

// vectors, points, matrices

// NIGHTMARE CODE!!!!!!  ANONYMOUS UNIONS ARE BAAAAAAAD THINGS MAN
// MLA - left in the #ifdef so the C++ compiler handles it, we just have to
// change references of g3s_vector.x to g3s_vector.xyz[0] (or .gX, .gY, etc.
// from the #defines)
typedef struct g3s_vector {
#if 0 // MLA #ifdef __cplusplus
	fix x,y,z;
#else
    union {
        struct {
            fix x, y, z;
        };
        fix xyz[3];
    };
#endif
} g3s_vector;

#define gX xyz[0]
#define gY xyz[1]
#define gZ xyz[2]

typedef struct g3s_angvec {
    fixang tx, ty, tz;
} g3s_angvec;

// This transformation matrix is row based, ie
//|m1 m2 m3|   |x|   |x'|
//|m4 m5 m6| * |y| = |y'|
//|m7 m8 m9|   |z|   |z'|
//
// but of course the incoming y coordinates
// are inverted, have a nice day
typedef struct g3s_matrix {
    fix m1, m2, m3, m4, m5, m6, m7, m8, m9;
} g3s_matrix;

// typedef short g3s_phandle;    //used to refer to points, equal to pntnum * 4
typedef struct g3s_point *g3s_phandle;

typedef struct g3s_point {
#if 0 // #ifdef __cplusplus
	fix x,y,z;
#else
    union { // rotated 3d coords, use as vector or elements
        g3s_vector vec;
        struct {
            fix x, y, z;
        };
        fix xyz[3];
        g3s_phandle next; // next in free list, when point is unused
    };
#endif

    fix sx, sy;     // screen coords
    ubyte codes;    // clip codes
    ubyte p3_flags; // misc flags
#if 0               // #ifdef __cplusplus
	fix16 u,v;
#else
    union {
        struct {
            fix16 u, v;
        } uv;        // for texturing, etc.
        grs_rgb rgb; // for RGB-space gouraud shading
    };
#endif
    fix16 i; // gouraud shading & lighting
} g3s_point;

// clip codes
#define CC_OFF_LEFT 1
#define CC_OFF_RIGHT 2
#define CC_OFF_BOT 4
#define CC_OFF_TOP 8
#define CC_BEHIND 128
#define CC_CLIP_OVERFLOW 16

// flags for the point structure
#define PF_U 1         // is u value used?
#define PF_V 2         // is v value used?
#define PF_I 4         // is i value used?
#define PF_PROJECTED 8 // has this point been projected?
#define PF_RGB 16      // are the RBG values used?
#define PF_CLIPPNT 32  // this point created by clipper
#define PF_LIT 64      // has this point been lit by the lighter?

// lighting codes
#define LT_NONE 0
#define LT_SOLID 1
#define LT_DIFF 2
#define LT_SPEC 4
#define LT_GOUR 128

#define LT_NEAR_LIGHT 16 // TRUE if light is near and has to be evaluated
#define LT_NEAR_VIEW 8   // TRUE if viewing point is near, and has to be reevaled
#define LT_LOC_LIGHT 32  // TRUE if light is a local point, not a vector
#define LT_TABSIZE 24    // size of the shading table
#define LT_BASELIT 15    // table entry of normal intensity (before saturating)

extern fix scrw, scrh;
extern fix biasx, biasy;

extern ubyte g3d_light_type;
extern fix g3d_amb_light, g3d_diff_light, g3d_spec_light;
extern fix g3d_ldotv, g3d_sdotl, g3d_sdotv, g3d_flash;
extern ubyte *g3d_light_tab;
extern g3s_vector g3d_light_src, g3d_light_trans;
extern g3s_vector g3d_view_vec, g3d_light_vec;

// DG: my compiler was not happy about the names "or" and "and", so I appended a _
typedef struct g3s_codes {
    byte or_;
    byte and_;
} g3s_codes;

/*
 *      We're going to want a bunch of general-purpose 3d vector math
 *      routines. These are:
 *
 *      Question:  some of these take a destination, and some change the
 *              vector passed to them. Should we adopt a consistent interface?
 *
 * There's sure to be more of these
 *
 */
void g3_vec_sub(g3s_vector *dest, g3s_vector *src1, g3s_vector *src2);
void g3_vec_add(g3s_vector *dest, g3s_vector *src1, g3s_vector *src2);

fix g3_vec_mag(g3s_vector *v);
void g3_vec_scale(g3s_vector *dest, g3s_vector *src, fix s);
void g3_vec_normalize(g3s_vector *v);

fix g3_vec_dotprod(g3s_vector *v0, g3s_vector *v1);

void g3_vec_rotate(g3s_vector *dest, g3s_vector *src, g3s_matrix *m);
// src and dest can be the same

void g3_transpose(g3s_matrix *m); // transpose in place
void g3_matrix_x_matrix(g3s_matrix *dest, g3s_matrix *src1, g3s_matrix *src2);

int g3_clip_line(g3s_point *src[], g3s_point *dest[]);
int g3_clip_polygon(int n, g3s_point *src[], g3s_point *dest[]);

/*
 *      Graphics-specific 3d routines
 *
 */

// System initialization, etc.

short g3_init(short max_points, int user_x_axis, int user_y_axis, int user_z_axis);
// the three axis vars describe your coordinate system.  Use the constants
//      X_AXIS,Y_AXIS,Z_AXIS, or negative of these, to describe what your
// coordinates mean. For each of width_,height_, and depth_axis, specify
//      which of your axes goes in that dimension. Depth is into the screen,
//      height is up, and width is to the right
// returns number actually allocated

void g3_shutdown(void); // frees allocated points, and whatever else
int g3_count_free_points(void);

// Point definition and manipulation

int g3_alloc_list(int n, g3s_phandle *p);
// allocates n points into p, returns 0 for none, or n for ok

g3s_phandle g3_rotate_point(g3s_vector *v);
// translate, rotate, and code point in 3-space. returns point handle

int g3_project_point(g3s_phandle p);
// project already-rotated point. returns true if z>0

g3s_phandle g3_transform_point(g3s_vector *v);
// translate, rotate, code, and project point (rotate_point + project_point);
// returns point handle

void g3_add_delta_x(g3s_phandle p, fix dx);
void g3_add_delta_z(g3s_phandle p, fix dz);
// adds a delta to a point

g3s_phandle g3_copy_add_delta_x(g3s_phandle src, fix dx);
g3s_phandle g3_copy_add_delta_y(g3s_phandle src, fix dy);
g3s_phandle g3_copy_add_delta_z(g3s_phandle src, fix dz);
g3s_phandle g3_copy_add_delta_xy(g3s_phandle src, fix dx, fix dy);
g3s_phandle g3_copy_add_delta_xz(g3s_phandle src, fix dx, fix dz);
g3s_phandle g3_copy_add_delta_yz(g3s_phandle src, fix dy, fix dz);
// adds a delta to a point, and stores in a new point

g3s_phandle g3_replace_add_delta_x(g3s_phandle src, g3s_phandle dst, fix dx);
g3s_phandle g3_replace_add_delta_y(g3s_phandle src, g3s_phandle dst, fix dy);
g3s_phandle g3_replace_add_delta_z(g3s_phandle src, g3s_phandle dst, fix dz);
// adds a delta to src and stores to preallocated point dst

// do a whole bunch of points. returns codes and & or
g3s_codes g3_transform_list(short n, g3s_phandle *dest_list, g3s_vector *v);

void g3_free_point(g3s_phandle p);               // adds to free list
void g3_free_list(int n_points, g3s_phandle *p); // adds to free list

// Frame setup commands

void g3_start_frame(void); // mark all points as unused
void g3_set_view_matrix(g3s_vector *pos, g3s_matrix *m, fix zoom);
void g3_set_view_angles(g3s_vector *pos, g3s_angvec *angles, int rotation_order, fix zoom); // takes ptr to angles

int g3_end_frame(void); // returns number of points lost. thus, 0==no error

// farms out a point based on flags
fix g3_light(g3s_phandle norm, g3s_phandle pos);

// evaluates light point relative to another point, uses light point
// this is only for local lighting, inside an object.  If its not local
// lighting, you don't need to call this one.  This assumes you'll want
// to set NEAR_LIGHT to zero.
void g3_eval_loc_light(g3s_phandle pos);

// takes the dot product of view and light, for specular light
void g3_eval_ldotv(void);

// evaluate the view vector relative to a point
void g3_eval_view(g3s_phandle pos);

// Misc commands

bool g3_check_normal_facing(g3s_vector *v, g3s_vector *normal);
// takes surface normal and unrotated point on poly. normal need not be normalized

bool g3_check_poly_facing(g3s_phandle p0, g3s_phandle p1, g3s_phandle p2);
// takes 3 rotated points on poly

fix g3_get_zoom(char axis, fixang angle, int window_width, int window_height);
// returns zoom factor to achieve the desired view angle. axis is 'y' or 'x'

void g3_get_view_pyramid(g3s_vector *corners);
// fills in 4 vectors, which unit vectors from the eye that describe the
// view pyramid.  first vector is upper right, then clockwise

void g3_get_slew_step(fix step_size, g3s_vector *x_step, g3s_vector *y_step, g3s_vector *z_step);
// fills in three vectors, each of length step_size, in the specified
// direction in the viewer's frame of reference.  any (or all) of the
// vector pointers can be NULL to skip that axis.

// Instancing. These all return true if everything ok

uchar g3_start_object_angles_xyz(g3s_vector *p, fixang tx, fixang ty, fixang tz, int rotation_order);

uchar g3_start_object_angles_x(g3s_vector *p, fixang tx);
uchar g3_start_object_angles_y(g3s_vector *p, fixang ty);
uchar g3_start_object_angles_z(g3s_vector *p, fixang tz);

// you can use this to scale things like make small boxes and the like.  The
// effect is to shrink or expand the points in their SOURCE coordinate system.
// Only call this after calling one of the start_object routines.  You can do it
// within a frame as well, the effect will be to make surrounding space smaller
// or bigger.  This will make you shoot towards or away from the origin,
// probably not the effect you're looking for
void g3_scale_object(fix s);

void g3_end_object(void);

int code_point(g3s_point *pt);

// Drawing commands.
// all return 2d clip codes. See 2d header for values

// note that for the 2 gouraud line drawers, the 3d assumes that the calling
// function has not only set the rgb or i fields of the passed points, but has
// also set the p3_flags byte to indicate which is being used.  'tis a terrible
// hack indeed (Spaz, 6/29)

int g3_draw_line(g3s_phandle p0, g3s_phandle p1);
int g3_draw_cline(g3s_phandle p0, g3s_phandle p1); // rgb-space gouraud line

int g3_draw_poly(long c, int n_verts, g3s_phandle *p);

int g3_draw_spoly(int n_verts, g3s_phandle *p); // smooth poly

int g3_draw_cpoly(int n_verts, g3s_phandle *p); // RBG-space smooth poly

int g3_check_and_draw_poly(long c, int n_verts, g3s_phandle *p);

int g3_check_and_draw_tluc_poly(long c, int n_verts, g3s_phandle *p);

int g3_check_and_draw_tluc_spoly(int n_verts, g3s_phandle *p);

grs_vertex **g3_light_bitmap(grs_bitmap *bm, g3s_phandle p);

grs_vertex **g3_light_anchor_bitmap(grs_bitmap *bm, g3s_phandle p, short u_anchor, short v_anchor);

void g3_set_bitmap_scale(fix u_scale, fix v_scale);

int g3_draw_tmap(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_light_tmap(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_draw_floor_map(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_light_floor_map(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_draw_wall_map(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_light_wall_map(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_draw_lmap(int n, g3s_phandle *vp, grs_bitmap *bm);
int g3_light_lmap(int n, g3s_phandle *vp, grs_bitmap *bm);

void g3_interpret_object(ubyte *object_ptr, ...);
extern void g3_set_tmaps_linear(void);
extern void g3_reset_tmaps(void);

#pragma pack(pop)

#if defined(__cplusplus)
}
#endif // !defined(__cplusplus)

#endif /* __3D_H */
