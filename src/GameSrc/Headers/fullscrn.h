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
#ifndef __FULLSCRN_H
#define __FULLSCRN_H

/*
 * $Source: n:/project/cit/src/inc/RCS/fullscrn.h $
 * $Revision: 1.4 $
 * $Author: tjs $
 * $Date: 1994/05/17 02:32:03 $
 *
 * $Log: fullscrn.h $
 * Revision 1.4  1994/05/17  02:32:03  tjs
 * Save\restore mfd in fullscreen fix.
 *
 * Revision 1.3  1994/03/04  07:05:02  mahk
 * Full screen mania.
 *
 * Revision 1.2  1993/09/22  00:46:57  xemu
 * added raising & lowering of invent * mfd regions
 *
 * Revision 1.1  1993/09/19  19:06:26  xemu
 * Initial revision
 *
 *
 */

// Includes
#include "cursors.h"
#include "frprotox.h"
#include "region.h"

// Defines

#define FULL_VIEW_X      0
#define FULL_VIEW_Y      0
#define FULL_VIEW_HEIGHT 200
#define FULL_VIEW_WIDTH  320

// Note, these have been kludged to parallel the
// view360 context numbers.
#define FULL_R_MFD_MASK   0x01
#define FULL_L_MFD_MASK   0x02
#define FULL_INVENT_MASK  0x04
#define FULL_MFD_MASK(id) (((id) == 0) ? FULL_L_MFD_MASK : FULL_R_MFD_MASK)

// Typedefs

// Prototypes
void change_svga_screen_mode();
void change_svga_cursors();
errtype fullscreen_init(void);
void fullscreen_start();
void fullscreen_exit(void);
errtype fullscreen_overlay();
errtype full_lower_region(LGRegion *r);
errtype full_raise_region(LGRegion *r);

// Globals
extern LGRegion *fullroot_region, *fullview_region;
extern LGRegion *inventory_region_full;
extern LGRegion *pagebutton_region_full;
extern uchar full_game_3d;
extern uchar full_visible;

extern uchar fullscrn_vitals;
extern uchar fullscrn_icons;

extern uiSlab fullscreen_slab;

#ifdef SVGA_SUPPORT
extern grs_screen *svga_screen;
extern frc *svga_render_context;
extern short svga_mode_data[];
//extern short mode_id;
#endif

#endif // __FULLSCRN_H
