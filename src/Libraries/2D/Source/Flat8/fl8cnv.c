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
 * $Source: r:/prj/lib/src/2d/RCS/fl8cnv.c $
 * $Revision: 1.78 $
 * $Author: kevin $
 * $Date: 1994/11/12 02:21:59 $
 *
 * General-purpose routines for drawing into straight-8 bitmaps.
 * This file is part of the 2d library.
 *
 */

#include "flat8.h"
#include "general.h"
#include "grnull.h"
#include "icanvas.h"

typedef void (*ptr_type)();

void (*flat8_canvas_table[GRD_CANVAS_FUNCS])() = {
    /* 8-bit pixel set/get */
    (ptr_type)flat8_set_upixel, // SET_UPIXEL8
    (ptr_type)gen_set_pixel,    // SET_PIXEL8
    (ptr_type)flat8_get_upixel, // GET_UPIXEL8
    (ptr_type)flat8_get_pixel,  // GET_PIXEL8
    /* 24-bit pixel set/get */
    (ptr_type)flat8_set_upixel24, // SET_UPIXEL24
    (ptr_type)flat8_set_pixel24,  // SET_PIXEL24
    (ptr_type)flat8_get_upixel24, // GET_UPIXEL24
    (ptr_type)flat8_get_pixel24,  // GET_PIXEL24
    /* integral, straight primitives */
    (ptr_type)flat8_clear,             // DRAW_CLEAR
    (ptr_type)temp_upoint,             // DRAW_UPOINT
    (ptr_type)temp_point,              // DRAW_POINT
    (ptr_type)flat8_set_upixel,        // SET_UPIXEL8_INTERRUPT
    (ptr_type)gen_set_pixel_interrupt, // SET_PIXEL8_INTERRUPT
    gr_null,                           // DRAW_UVLINE
    gr_null,                           // DRAW_VLINE
    (ptr_type)gen_urect,               // DRAW_URECT
    (ptr_type)gen_rect,                // DRAW_RECT
    (ptr_type)gen_ubox,                // DRAW_UBOX
    (ptr_type)gen_box,                 // DRAW_BOX
    /* states */
    gr_null, // PUSH_STATE
    gr_null, // POP_STATE
    /* fixed-point rendering primitives */
    gr_null,                         // FIX_USLINE
    gr_null,                         // FIX_SLINE
    gr_null,                         // FIX_UCLINE
    gr_null,                         // FIX_CLINE
    (ptr_type)temp_upoly,            // FIX_UPOLY
    (ptr_type)temp_poly,             // FIX_POLY
    (ptr_type)temp_uspoly,           // FIX_USPOLY
    (ptr_type)temp_spoly,            // FIX_SPOLY
    (ptr_type)temp_ucpoly,           // FIX_UCPOLY
    (ptr_type)temp_cpoly,            // FIX_CPOLY
    (ptr_type)temp_utpoly,           // FIX_TLUC8_UPOLY
    (ptr_type)temp_tpoly,            // FIX_TLUC8_POLY
    (ptr_type)temp_ustpoly,          // FIX_TLUC8_USPOLY
    (ptr_type)temp_stpoly,           // FIX_TLUC8_SPOLY
    (ptr_type)gen_vox_rect,          // VOX_RECT
    (ptr_type)gen_vox_poly,          // VOX_POLY
    (ptr_type)gen_vox_cpoly,         // VOX_CPOLY
    (ptr_type)flat8_interp2_ubitmap, // INTERP2_UBITMAP
    (ptr_type)flat8_filter2_ubitmap, // FILTER2_UBITMAP
    gr_not_imp,                      // ROLL_UBITMAP // (ptr_type) gen_roll_ubitmap,  MLA - not used?
    gr_not_imp,                      // ROLL_BITMAP  //   (ptr_type) gen_roll_bitmap,   MLA - not used?
    (ptr_type)temp_wall_umap,        // FLAT8_WALL_UMAP
    gr_null,                         // FLAT8_WALL_MAP
    (ptr_type)temp_lit_wall_umap,    // FLAT8_LIT_WALL_UMAP
    gr_null,                         // FLAT8_LIT_WALL_MAP
    (ptr_type)temp_clut_wall_umap,   // FLAT8_CLUT_WALL_UMAP
    gr_null,                         // FLAT8_CLUT_WALL_MAP
    (ptr_type)temp_floor_umap,       // FLAT8_FLOOR_UMAP
    gr_null,                         // FLAT8_FLOOR_MAP
    (ptr_type)temp_lit_floor_umap,   // FLAT8_LIT_FLOOR_UMAP
    gr_null,                         // FLAT8_LIT_FLOOR_MAP
    (ptr_type)temp_clut_floor_umap,  // FLAT8_CLUT_FLOOR_UMAP
    gr_null,                         // FLAT8_CLUT_FLOOR_MAP
    /* linear texture mappers */
    gr_null,                 // DEVICE_ULMAP
    gr_null,                 // DEVICE_LMAP
    gr_null,                 // MONO_ULMAP
    gr_null,                 // MONO_LMAP
    (ptr_type)temp_lin_umap, // FLAT8_ULMAP
    (ptr_type)temp_lin_map,  // FLAT8_LMAP
    (ptr_type)temp_lin_umap, // FLAT24_ULMAP
    (ptr_type)temp_lin_map,  // FLAT24_LMAP
    (ptr_type)temp_lin_umap, // RSD_ULMAP
    (ptr_type)temp_lin_map,  // RSD_LMAP
    (ptr_type)temp_lin_umap, // TLUC8_ULMAP
    (ptr_type)temp_lin_map,  // TLUC8_LMAP
    /* lit linear texture mappers */
    gr_null,                     // DEVICE_LIT_LIN_UMAP
    gr_null,                     // DEVICE_LIT_LIN_MAP
    gr_null,                     // MONO_LIT_LIN_UMAP
    gr_null,                     // MONO_LIT_LIN_MAP
    (ptr_type)temp_lit_lin_umap, // FLAT8_LIT_LIN_UMAP
    (ptr_type)temp_lit_lin_map,  // FLAT8_LIT_LIN_MAP
    gr_null,                     // FLAT24_LIT_LIN_UMAP
    gr_null,                     // FLAT24_LIT_LIN_MAP
    (ptr_type)temp_lit_lin_umap, // RSD_LIT_LIN_UMAP
    (ptr_type)temp_lit_lin_map,  // RSD_LIT_LIN_MAP
    gr_null,                     // TLUC8_LIT_LIN_UMAP
    gr_null,                     // TLUC8_LIT_LIN_MAP
    /* clut linear texture mappers */
    gr_null,                      // DEVICE_CLUT_LIN_UMAP
    gr_null,                      // DEVICE_CLUT_LIN_MAP
    gr_null,                      // MONO_CLUT_LIN_UMAP
    gr_null,                      // MONO_CLUT_LIN_MAP
    (ptr_type)temp_clut_lin_umap, // FLAT8_CLUT_LIN_UMAP
    (ptr_type)temp_clut_lin_map,  // FLAT8_CLUT_LIN_MAP
    gr_null,                      // FLAT24_CLUT_LIN_UMAP
    gr_null,                      // FLAT24_CLUT_LIN_MAP
    (ptr_type)temp_clut_lin_umap, // RSD_CLUT_LIN_UMAP
    (ptr_type)temp_clut_lin_map,  // RSD_CLUT_LIN_MAP
    (ptr_type)temp_clut_lin_umap, // TLUC8_CLUT_LIN_UMAP
    (ptr_type)temp_clut_lin_map,  // TLUC8_CLUT_LIN_MAP
    /* solid linear mapper */
    gr_null, // FLAT8_SOLID_LIN_UMAP
    gr_null, // FLAT8_SOLID_LIN_MAP
    /* perspective texture mappers */
    gr_null,                 // DEVICE_PER_UMAP
    gr_null,                 // DEVICE_PER_MAP
    gr_null,                 // MONO_PER_UMAP
    gr_null,                 // MONO_PER_MAP
    (ptr_type)temp_per_umap, // FLAT8_PER_UMAP
    (ptr_type)temp_per_map,  // FLAT8_PER_MAP
    gr_null,                 // FLAT24_PER_UMAP
    gr_null,                 // FLAT24_PER_MAP
    (ptr_type)temp_per_umap, // RSD_PER_UMAP
    (ptr_type)temp_per_map,  // RSD_PER_MAP
    gr_null,                 // TLUC8_PER_UMAP
    gr_null,                 // TLUC8_PER_MAP
    /* lit perspective texture mappers */
    gr_null,                     // DEVICE_LIT_PER_UMAP
    gr_null,                     // DEVICE_LIT_PER_MAP
    gr_null,                     // MONO_LIT_PER_UMAP
    gr_null,                     // MONO_LIT_PER_MAP
    (ptr_type)temp_lit_per_umap, // FLAT8_LIT_PER_UMAP
    gr_null,                     // FLAT8_LIT_PER_MAP
    gr_null,                     // FLAT24_LIT_PER_UMAP
    gr_null,                     // FLAT24_LIT_PER_MAP
    (ptr_type)temp_lit_per_umap, // RSD_LIT_PER_UMAP
    gr_null,                     // RSD_LIT_PER_MAP
    gr_null,                     // TLUC8_LIT_PER_UMAP
    gr_null,                     // TLUC8_LIT_PER_MAP
    /* clut perspective texture mappers */
    gr_null,                      // DEVICE_CLUT_PER_UMAP
    gr_null,                      // DEVICE_CLUT_PER_MAP
    gr_null,                      // MONO_CLUT_PER_UMAP
    gr_null,                      // MONO_CLUT_PER_MAP
    (ptr_type)temp_clut_per_umap, // FLAT8_CLUT_PER_UMAP
    (ptr_type)temp_clut_per_map,  // FLAT8_CLUT_PER_MAP
    gr_null,                      // FLAT24_CLUT_PER_UMAP
    gr_null,                      // FLAT24_CLUT_PER_MAP
    (ptr_type)temp_clut_per_umap, // RSD_CLUT_PER_UMAP
    (ptr_type)temp_clut_per_map,  // RSD_CLUT_PER_MAP
    gr_null,                      // TLUC8_CLUT_PER_UMAP
    gr_null,                      // TLUC8_CLUT_PER_MAP
    /* solid perspective mapper */
    gr_null, // FLAT8_SOLID_PER_UMAP
    gr_null, // FLAT8_SOLID_PER_MAP
    /* curves, should change to fixed-point */
    (ptr_type)gen_int_ucircle, // INT_UCIRCLE
    (ptr_type)gen_int_circle,  // INT_CIRCLE
    gr_null,                   // FIX_UCIRCLE
    gr_null,                   // FIX_CIRCLE
    (ptr_type)gen_int_udisk,   // INT_UDISK
    (ptr_type)gen_int_disk,    // INT_DISK
    gr_null,                   // FIX_UDISK
    gr_null,                   // FIX_DISK
    gr_null,                   // INT_UROD
    gr_null,                   // INT_ROD
    gr_null,                   // FIX_UROD
    gr_null,                   // FIX_ROD
    /* bitmap drawing functions. */
    // MLA - added these two for the device functions
    (ptr_type)flat8_flat8_ubitmap, // DRAW_DEVICE_UBITMAP
    (ptr_type)gen_flat8_bitmap,    // DRAW_DEVICE_BITMAP
    (ptr_type)flat8_mono_ubitmap,  // DRAW_MONO_UBITMAP
    (ptr_type)gen_mono_bitmap,     // DRAW_MONO_BITMAP
    (ptr_type)temp_flat8_ubitmap,  // DRAW_FLAT8_UBITMAP
    (ptr_type)gen_flat8_bitmap,    // DRAW_FLAT8_BITMAP
    (ptr_type)gen_flat24_ubitmap,  // DRAW_FLAT24_UBITMAP
    (ptr_type)gen_flat24_bitmap,   // DRAW_FLAT24_BITMAP
    (ptr_type)temp_rsd8_ubitmap,   // DRAW_RSD8_UBITMAP
    (ptr_type)temp_rsd8_bitmap,    // DRAW_RSD8_BITMAP
    (ptr_type)temp_tluc8_ubitmap,  // DRAW_TLUC8_UBITMAP
    (ptr_type)gen_tluc8_bitmap,    // DRAW_TLUC8_BITMAP
    /* bitmap drawing functions through a clut. */
    gr_null,                            // CLUT_DRAW_DEVICE_UBITMAP
    gr_null,                            // CLUT_DRAW_DEVICE_BITMAP
    gr_null,                            // CLUT_DRAW_MONO_UBITMAP
    gr_null,                            // CLUT_DRAW_MONO_BITMAP
    (ptr_type)temp_flat8_clut_ubitmap,  // CLUT_DRAW_FLAT8_UBITMAP
    (ptr_type)gen_flat8_clut_bitmap,    // CLUT_DRAW_FLAT8_BITMAP
    gr_null,                            // CLUT_DRAW_FLAT24_UBITMAP
    gr_null,                            // CLUT_DRAW_FLAT24_BITMAP
    (ptr_type)unpack_rsd8_clut_ubitmap, // CLUT_DRAW_RSD8_UBITMAP
    (ptr_type)unpack_rsd8_clut_bitmap,  // CLUT_DRAW_RSD8_BITMAP
    gr_null,                            // CLUT_DRAW_TLUC8_UBITMAP
    gr_null,                            // CLUT_DRAW_TLUC8_BITMAP
    /* rsd8 solid bitmap functions.  No longer used. */
    gr_null, // SOLID_RSD8_UBITMAP
    gr_null, // SOLID_RSD8_BITMAP
    /* scaled bitmap drawing functions */
    gr_null,                            // SCALE_DEVICE_UBITMAP
    gr_null,                            // SCALE_DEVICE_BITMAP
    (ptr_type)flat8_mono_scale_ubitmap, // SCALE_MONO_UBITMAP
    (ptr_type)flat8_mono_scale_bitmap,  // SCALE_MONO_BITMAP
    (ptr_type)temp_scale_umap,          // SCALE_FLAT8_UBITMAP
    (ptr_type)temp_scale_map,           // SCALE_FLAT8_BITMAP
    gr_null,                            // SCALE_FLAT24_UBITMAP
    gr_null,                            // SCALE_FLAT24_BITMAP
    (ptr_type)temp_scale_umap,          // SCALE_RSD8_UBITMAP
    (ptr_type)temp_scale_map,           // SCALE_RSD8_BITMAP
    (ptr_type)temp_scale_umap,          // SCALE_TLUC8_UBITMAP
    (ptr_type)temp_scale_map,           // SCALE_TLUC8_BITMAP
    /* rsd8 solid scale functions.  No longer used. */
    gr_null, // SOLID_SCALE_RSD8_UBITMAP
    gr_null, // SOLID_SCALE_RSD8_BITMAP
    /* bitmap scaling functions through a clut */
    gr_null,                        // CLUT_SCALE_DEVICE_UBITMAP
    gr_null,                        // CLUT_SCALE_DEVICE_BITMAP
    gr_null,                        // CLUT_SCALE_MONO_UBITMAP
    gr_null,                        // CLUT_SCALE_MONO_BITMAP
    (ptr_type)temp_clut_scale_umap, // CLUT_SCALE_FLAT8_UBITMAP
    (ptr_type)temp_clut_scale_map,  // CLUT_SCALE_FLAT8_BITMAP
    gr_null,                        // CLUT_SCALE_FLAT24_UBITMAP
    gr_null,                        // CLUT_SCALE_FLAT24_BITMAP
    (ptr_type)temp_clut_scale_umap, // CLUT_SCALE_RSD8_UBITMAP
    (ptr_type)temp_clut_scale_map,  // CLUT_SCALE_RSD8_BITMAP
    (ptr_type)temp_clut_scale_umap, // CLUT_SCALE_TLUC8_UBITMAP
    (ptr_type)temp_clut_scale_map,  // CLUT_SCALE_TLUC8_BITMAP
    /* bitmap mask draw functions. */
    gr_null,                          // MASK_DEVICE_UBITMAP
    gr_null,                          // MASK_DEVICE_BITMAP
    gr_null,                          // MASK_MONO_UBITMAP
    gr_null,                          // MASK_MONO_BITMAP
    (ptr_type)flat8_flat8_ubitmap,    // MASK_FLAT8_UBITMAP
    (ptr_type)temp_flat8_mask_bitmap, // MASK_FLAT8_BITMAP
    gr_null,                          // MASK_FLAT24_UBITMAP
    gr_null,                          // MASK_FLAT24_BITMAP
    gr_null,                          // MASK_RSD8_UBITMAP
    gr_null,                          // MASK_RSD8_BITMAP
    gr_null,                          // MASK_TLUC8_UBITMAP
    gr_null,                          // MASK_TLUC8_BITMAP
    /* bitmap get functions. */
    gr_null,                           // GET_DEVICE_UBITMAP
    gr_null,                           // GET_DEVICE_BITMAP
    gr_null,                           // GET_MONO_UBITMAP
    gr_null,                           // GET_MONO_BITMAP
    (ptr_type)flat8_get_flat8_ubitmap, // GET_FLAT8_UBITMAP
    (ptr_type)gen_get_flat8_bitmap,    // GET_FLAT8_BITMAP
    gr_null,                           // GET_FLAT24_UBITMAP
    gr_null,                           // GET_FLAT24_BITMAP
    gr_null,                           // GET_RSD8_UBITMAP
    gr_null,                           // GET_RSD8_BITMAP
    gr_null,                           // GET_TLUC8_UBITMAP
    gr_null,                           // GET_TLUC8_BITMAP
    /* bitmap horizontal flip functions */
    gr_null,                             // HFLIP_DEVICE_UBITMAP
    gr_null,                             // HFLIP_DEVICE_BITMAP
    gr_null,                             // HFLIP_MONO_UBITMAP
    gr_null,                             // HFLIP_MONO_BITMAP
    (ptr_type)flat8_hflip_flat8_ubitmap, // HFLIP_FLAT8_UBITMAP
    (ptr_type)gen_hflip_flat8_bitmap,    // HFLIP_FLAT8_BITMAP
    gr_null,                             // HFLIP_FLAT24_UBITMAP
    gr_null,                             // HFLIP_FLAT24_BITMAP
    gr_null,                             // HFLIP_RSD8_UBITMAP
    gr_null,                             // HFLIP_RSD8_BITMAP
    gr_null,                             // HFLIP_TLUC8_UBITMAP
    gr_null,                             // HFLIP_TLUC8_BITMAP
    /* bitmap color lookup table horizontal flip functions */
    gr_null,                                  // CLUT_HFLIP_DEVICE_UBITMAP
    gr_null,                                  // CLUT_HFLIP_DEVICE_BITMAP
    gr_null,                                  // CLUT_HFLIP_MONO_UBITMAP
    gr_null,                                  // CLUT_HFLIP_MONO_BITMAP
    (ptr_type)flat8_clut_hflip_flat8_ubitmap, // CLUT_HFLIP_FLAT8_UBITMAP
    (ptr_type)gen_clut_hflip_flat8_bitmap,    // CLUT_HFLIP_FLAT8_BITMAP
    gr_null,                                  // CLUT_HFLIP_FLAT24_UBITMAP
    gr_null,                                  // CLUT_HFLIP_FLAT24_BITMAP
    gr_null,                                  // CLUT_HFLIP_RSD8_UBITMAP
    gr_null,                                  // CLUT_HFLIP_RSD8_BITMAP
    gr_null,                                  // CLUT_HFLIP_TLUC8_UBITMAP
    gr_null,                                  // CLUT_HFLIP_TLUC8_BITMAP
    /* bitmap horizontal doubling. */
    gr_null,                                // DOUBLE_H_DEVICE_UBITMAP
    gr_null,                                // DOUBLE_H_DEVICE_BITMAP
    gr_null,                                // DOUBLE_H_MONO_UBITMAP
    gr_null,                                // DOUBLE_H_MONO_BITMAP
    (ptr_type)flat8_flat8_h_double_ubitmap, // DOUBLE_H_FLAT8_UBITMAP
    gr_null,                                // DOUBLE_H_FLAT8_BITMAP
    gr_null,                                // DOUBLE_H_FLAT24_UBITMAP
    gr_null,                                // DOUBLE_H_FLAT24_BITMAP
    gr_null,                                // DOUBLE_H_RSD8_UBITMAP
    gr_null,                                // DOUBLE_H_RSD8_BITMAP
    gr_null,                                // DOUBLE_H_TLUC8_UBITMAP
    gr_null,                                // DOUBLE_H_TLUC8_BITMAP
    /* bitmap vertical doubling. */
    gr_null,                                // DOUBLE_V_DEVICE_UBITMAP
    gr_null,                                // DOUBLE_V_DEVICE_BITMAP
    gr_null,                                // DOUBLE_V_MONO_UBITMAP
    gr_null,                                // DOUBLE_V_MONO_BITMAP
    (ptr_type)flat8_flat8_v_double_ubitmap, // DOUBLE_V_FLAT8_UBITMAP
    gr_null,                                // DOUBLE_V_FLAT8_BITMAP
    gr_null,                                // DOUBLE_V_FLAT24_UBITMAP
    gr_null,                                // DOUBLE_V_FLAT24_BITMAP
    gr_null,                                // DOUBLE_V_RSD8_UBITMAP
    gr_null,                                // DOUBLE_V_RSD8_BITMAP
    gr_null,                                // DOUBLE_V_TLUC8_UBITMAP
    gr_null,                                // DOUBLE_V_TLUC8_BITMAP
    /* bitmap horizontal and vertical doubling. */
    gr_null, // DOUBLE_HV_DEVICE_UBITMAP
    gr_null, // DOUBLE_HV_DEVICE_BITMAP
    gr_null, // DOUBLE_HV_MONO_UBITMAP
    gr_null, // DOUBLE_HV_MONO_BITMAP
    gr_null, // DOUBLE_HV_FLAT8_UBITMAP
    gr_null, // DOUBLE_HV_FLAT8_BITMAP
    gr_null, // DOUBLE_HV_FLAT24_UBITMAP
    gr_null, // DOUBLE_HV_FLAT24_BITMAP
    gr_null, // DOUBLE_HV_RSD8_UBITMAP
    gr_null, // DOUBLE_HV_RSD8_BITMAP
    gr_null, // DOUBLE_HV_TLUC8_UBITMAP
    gr_null, // DOUBLE_HV_TLUC8_BITMAP
    /* bitmap smooth horizontal doubling. */
    gr_null,                                       // SMOOTH_DOUBLE_H_DEVICE_UBITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_DEVICE_BITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_MONO_UBITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_MONO_BITMAP
    (ptr_type)flat8_flat8_smooth_h_double_ubitmap, // SMOOTH_DOUBLE_H_FLAT8_UBITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_FLAT8_BITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_FLAT24_UBITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_FLAT24_BITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_RSD8_UBITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_RSD8_BITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_TLUC8_UBITMAP
    gr_null,                                       // SMOOTH_DOUBLE_H_TLUC8_BITMAP
    /* bitmap smooth vertical doubling. */
    gr_null, // SMOOTH_DOUBLE_V_DEVICE_UBITMAP
    gr_null, // SMOOTH_DOUBLE_V_DEVICE_BITMAP
    gr_null, // SMOOTH_DOUBLE_V_MONO_UBITMAP
    gr_null, // SMOOTH_DOUBLE_V_MONO_BITMAP
    gr_null, // SMOOTH_DOUBLE_V_FLAT8_UBITMAP
    gr_null, // SMOOTH_DOUBLE_V_FLAT8_BITMAP
    gr_null, // SMOOTH_DOUBLE_V_FLAT24_UBITMAP
    gr_null, // SMOOTH_DOUBLE_V_FLAT24_BITMAP
    gr_null, // SMOOTH_DOUBLE_V_RSD8_UBITMAP
    gr_null, // SMOOTH_DOUBLE_V_RSD8_BITMAP
    gr_null, // SMOOTH_DOUBLE_V_TLUC8_UBITMAP
    gr_null, // SMOOTH_DOUBLE_V_TLUC8_BITMAP
    /* bitmap smooth horizontal and vertical doubling. */
    gr_null,                                        // SMOOTH_DOUBLE_HV_DEVICE_UBITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_DEVICE_BITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_MONO_UBITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_MONO_BITMAP
    (ptr_type)flat8_flat8_smooth_hv_double_ubitmap, // SMOOTH_DOUBLE_HV_FLAT8_UBITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_FLAT8_BITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_FLAT24_UBITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_FLAT24_BITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_RSD8_UBITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_RSD8_BITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_TLUC8_UBITMAP
    gr_null,                                        // SMOOTH_DOUBLE_HV_TLUC8_BITMAP
    /* text/font functions. */
    (ptr_type)gen_font_ustring,       // DRAW_USTRING
    (ptr_type)gen_font_string,        // DRAW_STRING
    (ptr_type)gen_font_scale_ustring, // DRAW_SCALE_USTRING
    (ptr_type)gen_font_scale_string,  // DRAW_SCALE_STRING
    (ptr_type)gen_font_uchar,         // DRAW_UCHAR
    (ptr_type)gen_font_char,          // DRAW_CHAR
    /* utility functions; bitmap type specific functions */
    (ptr_type)flat8_calc_row,   // CALC_ROW
    (ptr_type)flat8_sub_bitmap, // SUB_BITMAP
    /* placeholders for primitiveless chains */
    gr_null, // START_FRAME,
    gr_null, // END_FRAME
};
