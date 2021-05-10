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
#ifndef __RENDER_H
#define __RENDER_H

/*
 * $Source: n:/project/cit/src/inc/RCS/render.h $
 * $Revision: 1.16 $
 * $Author: xemu $
 * $Date: 1994/03/20 21:16:13 $
 *
 *
 */

// Includes
#include "objects.h"
#include "rect.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
#define TM_SIZE_CNT 3

#define MAX_CAMERAS_VISIBLE 2
#define NUM_HACK_CAMERAS 8
// hack cameras are "custom textures" 7c through 7f
// (so, factoring in type, the low byte is fc to ff
//#define FIRST_CAMERA_TMAP     (short)0x80 - NUM_HACK_CAMERAS
#define FIRST_CAMERA_TMAP 0x78

// Prototypes
errtype render_run(void);

// Globals
extern LGRect *rendrect;

extern uchar fr_texture;
extern uchar fr_txt_walls;
extern uchar fr_txt_floors;
extern uchar fr_txt_ceilings;
extern uchar fr_lighting, fr_play_lighting, fr_lights_out, fr_normal_lights;
extern int fr_detail_value;
extern int fr_drop[TM_SIZE_CNT];
extern uchar fr_show_tilecursor;
extern uchar fr_cont_tilecursor;
extern uchar fr_show_all;
extern int fr_qscale_crit, fr_qscale_obj;
extern uchar fr_highlights;
extern int fr_lite_rad1, fr_lite_base1, fr_lite_rad2, fr_lite_base2;
extern int fr_normal_shf;
extern fix fr_lite_slope, fr_lite_yint;
extern int fr_detail_master;
extern int fr_pseudo_spheres;

extern grs_bitmap *static_bitmap;

extern uchar hack_cameras_needed;
extern uchar hack_takeover;
extern grs_bitmap *hack_cam_bitmaps[MAX_CAMERAS_VISIBLE];
extern char camera_map[NUM_HACK_CAMERAS];
extern ObjID hack_cam_objs[NUM_HACK_CAMERAS];
extern ObjID hack_cam_surrogates[NUM_HACK_CAMERAS];

errtype init_hack_cameras(void);
errtype shutdown_hack_cameras(void);
errtype do_screen_static(void);
errtype render_hack_cameras(void);
errtype hack_camera_takeover(int hack_cam);
errtype hack_camera_relinquish(void);

void tile_hit(int mx, int my);

#endif // __RENDER_H
