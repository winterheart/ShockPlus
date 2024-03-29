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
 * $Source: r:/prj/cit/src/RCS/fullamap.c $
 * $Revision: 1.11 $
 * $Author: tjs $
 * $Date: 1994/11/09 11:23:27 $
 *
 */

#include "Shock.h"
#include "tools.h"
#include "game_screen.h"
#include "input.h"

#include "mainloop.h"
#include "amaploop.h"
#include "lvldata.h"

// -------------------
//  INTERNAL PROTOTYPES
// -------------------
uchar amap_mouse_handler(uiEvent *ev, LGRegion *, intptr_t);
uchar amap_key_handler(uiEvent *ev, LGRegion *r, intptr_t user_data);
errtype amap_init(void);

uchar amap_mouse_handler(uiEvent *ev, LGRegion *reg, intptr_t v) {
    uiMouseData *md = &ev->mouse_data;
    if (md->action & (MOUSE_LDOWN | MOUSE_LUP | MOUSE_WHEELUP | MOUSE_WHEELDN))
        return amap_ms_callback(oAMap(MFD_FULLSCR_MAP), ev->pos.x, ev->pos.y, md->action, md->buttons);
    return (TRUE);
}

/// AMap Keyboard event handler
uchar amap_key_handler(uiEvent *ev, LGRegion *r, intptr_t user_data) {
    if (amap_kb_callback(oAMap(MFD_FULLSCR_MAP), &ev->sdl_data))
        return FALSE;
    return (main_kb_callback(ev, r, user_data));
}

// -------------------------------------------------------------
// amap_init()
// This gets called at the very beginning of time

uiSlab amap_slab;
LGRegion amap_root_region;

errtype amap_init(void) {
    int id;
    LGRect mac_rect = {{0, 0}, {640, 480}};

    generic_reg_init(TRUE, &amap_root_region, &mac_rect, &amap_slab, amap_key_handler, amap_mouse_handler);
    uiInstallRegionHandler(&amap_root_region, UI_EVENT_KBD_POLL | UI_EVENT_MOUSE, amap_scroll_handler, 0, &id);
    return (OK);
}

// -------------------------------------------------------------
// amap_start()
// This gets called when we actually enter into the amap loop

void amap_start() {
#ifdef GADGET
    _current_root = NULL; /* got rid of pointer type mismatch
                           * since one was a region and the other a gadget
                           * someone should probably go and figure it out
                           */
#endif
    current_3d_flag = ANIM_UPDATE;
    _current_fr_context = NULL;
    _current_view = &amap_root_region;
    static_change_copy();
    message_info("");

    hotKeyDispatcher.setContext(ShockPlus::Contexts::AMAP_CONTEXT);
    uiSetCurrentSlab(&amap_slab);

    gr_set_screen(svga_screen);
    fsmap_startup();
    uiShowMouse(NULL);
}

// -----------------------------------------------
// amap_exit()
// This gets called when we leave amap mode

void amap_exit() {
    fsmap_free();
    uiHideMouse(NULL);
    gr_set_screen(cit_screen);
}
