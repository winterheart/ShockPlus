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
// $Source: r:/prj/lib/src/3d/RCS/globv.asm $
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

g3s_point *point_list = 0; //   dd      0       ;ptr to point buffer
short n_points = 0;        //   dw      0       ;num points allocated
g3s_point *first_free = 0; //   dd      0       ;ptr to first free pnt

g3s_matrix unscaled_matrix; // g3s_matrix <>   ;unscaled & unadjusted

// note: view_matrix and view_position must remain in this order!
g3s_matrix view_matrix;    // g3s_matrix <>
g3s_vector _view_position; // g3s_vector <>
fix _view_zoom;            // fix     ?
fix view_heading;          // fix     ?
fix view_pitch;            // fix     ?
fix view_bank;             // fix     ?

// are to save inverse object to world matrix and position
// to go from world to object, take Ax + a (like in real 3d)
g3s_matrix _wtoo_matrix;   //  g3s_matrix <>
g3s_vector _wtoo_position; //  g3s_vector <>

fix pixel_ratio; //  fix     ?       ;copy from 2d drv_cap

long window_width;  //  dd      ?
long window_height; //  dd      ?

long ww2; //  dd      ?       ;one-half widht,height
long wh2; //  dd      ?       ;..for texture mapper

long _scrw; //  dd      ?       ;need to do double-word mul
long _scrh; //	dd      ?

fix _biasx; //	fix     ?
fix _biasy; // fix     ?

g3s_vector _matrix_scale;  //  <>   ;how the columns are scaled
g3s_vector horizon_vector; //  <>   ;info for drawing the horizon

// clang-format off
// this tables tells you many bits to shift to get zero
uchar shift_table[256] = {
    0,
    1,
    2, 2,
    3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};
// clang-format on

// these vars describe the translation from the user's coordinate system
// to our coordinate system

long up_axis; // dd      ?

// which axis is our x,y,z?
long axis_x; //  dd      ?
long axis_z; //  dd      ?
long axis_y; //  dd      ?

// offset into matrix of axis which is x,y,z
long axis_x_ofs; //      dd      ?
long axis_z_ofs; //      dd      ?
long axis_y_ofs; //      dd      ?

char axis_swap_flag; // 			db      ?
char axis_neg_flag;  //			db      ?

// Lighting globals
char _g3d_light_type = 0; // db      0       ; The lighting type, see above

fix _g3d_amb_light = 0;        // fix    0       ; amount of ambient light
fix _g3d_diff_light = 0x10000; // fix    10000h  ; intensity of light source
fix _g3d_spec_light = 0;       // fix    0       ; amount of spec light
fix _g3d_flash = 0;            // fix    0       ; specular flash point below which none is applied
// note that specular light is used to provide a "flash" mostly
// so you can artificially inflate it a bit, or we could try
// funny functions to make it "flash" only within a certain
// range.

g3s_vector _g3d_light_src;   //  g3s_vector      <>      ; light source, either
                             //  local or vector
g3s_vector _g3d_light_trans; //  g3s_vector   		<>      ; point
                             //  source in view coords
g3s_vector _g3d_light_vec;   //  g3s_vector      <>      ; current light vector,
                             //  computed from src and flag

g3s_vector _g3d_view_vec; //  g3s_vector      <>      ; current viewing vector,
                          //  may have to be computed periodically

fix _g3d_ldotv; //  fix     ?       ; light vector dotted with view vector (for
                //  specular only)
fix _g3d_sdotl; //  fix     ?       ; surface vector dotted with light vector
                //  (for diffuse and spec)
fix _g3d_sdotv; //  fix     ?       ; surface vector dotted with view vector
                //  (ostensibly jnorm)

long _g3d_light_tab = 0; //  dd      0       ; lighting table with 32 or 24
                         //  entries.  Should go from black to white,

// palette base for gouraud-shaded polys

fix16 gouraud_base; //  sfix    ?
