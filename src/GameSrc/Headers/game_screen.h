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
#ifndef __GAMESCREEN_H
#define __GAMESCREEN_H

/*
 * $Source: n:/project/cit/src/inc/RCS/screen.h $
 * $Revision: 1.20 $
 * $Author: dc $
 * $Date: 1994/05/11 21:42:04 $
 *
 *
 */

#define GADGET_GAMESCREEN

// C Library Includes

// System Library Includes
#include "event.h"
#include "frprotox.h"

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
#define SCREEN_VIEW_X      28
#define SCREEN_VIEW_Y      24
#define SCREEN_VIEW_HEIGHT 108
#define SCREEN_VIEW_WIDTH  268

// Prototypes

// Initialize the main game screen.  This should only be called once, at
// the major initialization stage of the program.
errtype screen_init(void);

// Bring the main game screen to the monitor, doing appropriate cool
// looking tricks (palette fading, etc.) as necessary
void screen_start(void);

// Do appropriate stuff to indicate that we have left the game screen
void screen_exit(void);

// Force a draw of the whole durned screen
errtype screen_draw(void);

// Stop doing graphics things
errtype screen_shutdown(void);

// Handle keyboard input anywhere on the main screen
uchar main_kb_callback(uiEvent *h, LGRegion *r, intptr_t udata);

// rct NULL means fullscreen, slb NULL is no slabinit, key and maus are callbacks, NULL is no install
void generic_reg_init(uchar create_reg, LGRegion *reg, LGRect *rct, uiSlab *slb, uiHandlerProc key_h, uiHandlerProc maus_h);

// Globals
extern uchar *default_font_buf;
extern LGRegion *root_region, *mainview_region, *inventory_region_game, *status_region;
extern LGRegion *pagebutton_region_game;
extern LGCursor globcursor;
extern LGCursor wait_cursor;
extern LGCursor fire_cursor;
extern LGCursor vmail_cursor;
extern frc *normal_game_fr_context;

extern uiSlab main_slab;
extern LGRect fscrn_rect;
extern LGRect mainview_rect;

#endif // __GAMESCREEN_H
