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
#ifndef __INVENT_H
#define __INVENT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/invent.h $
 * $Revision: 1.14 $
 * $Author: xemu $
 * $Date: 1994/07/18 21:31:31 $
 *
 */

// Includes
#include "cursors.h"
#include "invdims.h"

// C Library Includes

// System Library Includes
#include "objects.h"

// Master Game Includes

// Game Library Includes

// Game Object Includes

#define MAX_GENERAL_INVENTORY 12

// Prototypes

// creates and initializes the inventory region
LGRegion *create_invent_region(LGRegion *parent, LGRegion **pbuttons, LGRegion **pinvent);

// Draw the inventory area.  Keeps information on most recent draw so only does
// incremental updates.
errtype inventory_draw(void);

// Force the inventory panel to draw, no matter what
errtype inventory_full_redraw(void);

// switch the inventory page to pgnum and redraw
errtype inventory_draw_new_page(int pgnum);

// clears the inventory region
errtype inventory_clear(void);

// Add the appropriate kind of object to the player's inventory.   Returns whether
// or not the action succeded (typical failure reason being not enough inventory
// slots remaining).
uchar inventory_add_object(ObjID new_object, uchar select);

// Removes the specified object from the player's inventory.  Does not do any
// correlation with the rest of the Universe -- this needs to be handed by
// the dropping/consuming/destroying code.
errtype inventory_remove_object(ObjID new_object);

void draw_page_buttons(uchar full);

void inv_change_fullscreen(uchar on);
void inv_update_fullscreen(uchar full);

errtype inventory_update_screen_mode();

void push_inventory_cursors(LGCursor *newcurs);
void pop_inventory_cursors(void);

void super_drop_func(int dispnum, int row);
void super_use_func(int dispnum, int row);

void absorb_object_on_cursor(ushort keycode, uint32_t context, intptr_t data);

uchar cycle_weapons_func(ushort keycode, uint32_t context, intptr_t data);

void push_live_grenade_cursor(ObjID obj);

void set_current_active(int activenum);

void remove_general_item(ObjID obj);

void add_email_datamunge(short mung, uchar select);

void invent_language_change(void);

// Globals
extern short inventory_page;
extern short inv_last_page;
extern int last_invent_cnum;
extern uchar show_all_actives;

extern uchar dirty_inv_canvas;

extern grs_bitmap inv_backgnd;
extern grs_canvas inv_norm_canvas;
extern grs_canvas inv_fullscrn_canvas;
extern grs_canvas inv_view360_canvas;
extern grs_canvas *pinv_canvas;

extern grs_canvas inv_gamepage_canvas;
extern grs_canvas inv_fullpage_canvas;
extern grs_canvas *ppage_canvas;

extern LGRegion *inventory_region;
extern LGRegion *pagebutton_region;


#endif // __INVENT_H
