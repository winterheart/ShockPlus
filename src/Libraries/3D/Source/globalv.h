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
// $Source: r:/prj/lib/src/3d/RCS/globv.h $
// $Revision: 1.5 $
// $Author: jaemz $
// $Date: 1994/11/06 19:10:28 $
//
// Global vars for the 3d system
//
// $Log: globv.asm $
// Revision 1.5  1994/11/06  19:10:28  jaemz
// Allow stereo to be externed even in non stereo version to test
//
// Revision 1.4  1994/09/28  19:01:00  jaemz
// Fixed stereo bug
//
// Revision 1.3  1994/09/20  13:29:08  jaemz
// Added globals for lighting
//
// Revision 1.2  1994/08/18  03:46:56  jaemz
// Changed stereo glob names to have underscore for c
//
// Revision 1.1  1994/08/04  17:47:21  jaemz
// Initial revision
//
//

#include "3d.h"
#include "lg.h"

// point allocation vars

extern g3s_point *point_list; //   dd      0       ;ptr to point buffer
extern short n_points;        //   dw      0       ;num points allocated
extern g3s_point *first_free; //   dd      0       ;ptr to first free pnt

extern g3s_matrix unscaled_matrix; // g3s_matrix <>   ;unscaled & unadjusted

// note: view_matrix and view_position must remain in this order!
extern g3s_matrix view_matrix;    // g3s_matrix <>
extern g3s_vector view_position; // g3s_vector <>
extern fix view_zoom;            // fix     ?
extern fix view_heading;          // fix     ?
extern fix view_pitch;            // fix     ?
extern fix view_bank;             // fix     ?

// are to save inverse object to world matrix and position
// to go from world to object, take Ax + a (like in real 3d)
extern g3s_matrix wtoo_matrix;   //  g3s_matrix <>
extern g3s_vector wtoo_position; //  g3s_vector <>

extern fix pixel_ratio; //  fix     ?       ;copy from 2d drv_cap

extern int32_t window_width;  //  dd      ?
extern int32_t window_height; //  dd      ?

extern int32_t ww2; //  dd      ?       ;one-half widht,height
extern int32_t wh2; //  dd      ?       ;..for texture mapper

extern int32_t scrw; //  dd      ?       ;need to do double-word mul
extern int32_t scrh; //	dd      ?

extern fix biasx; //	fix     ?
extern fix biasy; // fix     ?

extern g3s_vector matrix_scale;  // how the columns are scaled
extern g3s_vector horizon_vector; // info for drawing the horizon

// this tables tells you many bits to shift to get zero
extern uchar shift_table[];
extern int32_t up_axis; // dd      ?

// which axis is our x,y,z?
extern int32_t axis_x; //  dd      ?
extern int32_t axis_z; //  dd      ?
extern int32_t axis_y; //  dd      ?

// offset into matrix of axis which is x,y,z
extern int32_t axis_x_ofs; //      dd      ?
extern int32_t axis_z_ofs; //      dd      ?
extern int32_t axis_y_ofs; //      dd      ?

extern char axis_swap_flag; // 			db      ?
extern char axis_neg_flag;  //			db      ?

// Lighting globals
extern char g3d_light_type; // The lighting type, see above

extern fix g3d_amb_light;  // amount of ambient light
extern fix g3d_diff_light; // intensity of light source
extern fix g3d_spec_light; // amount of spec light
extern fix g3d_flash;      // specular flash point below which none is applied
// note that specular light is used to provide a "flash" mostly
// so you can artificially inflate it a bit, or we could try
// funny functions to make it "flash" only within a certain
// range.

extern g3s_vector g3d_light_src;   // light source, either local or vector
extern g3s_vector g3d_light_trans; // point source in view coords
extern g3s_vector g3d_light_vec;   // current light vector, computed from src and flag

extern g3s_vector g3d_view_vec; // current viewing vector, may have to be computed periodically

extern fix g3d_ldotv; // light vector dotted with view vector (for specular only)
extern fix g3d_sdotl; // surface vector dotted with light vector (for diffuse and spec)
extern fix g3d_sdotv; // surface vector dotted with view vector (ostensibly jnorm)

extern int32_t g3d_light_tab; // lighting table with 32 or 24 entries. Should go from black to white,

// palette base for gouraud-shaded polys

extern fix16 gouraud_base; //  sfix    ?
