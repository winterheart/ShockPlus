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
 * $Source: n:/project/lib/src/ui/RCS/curdrw.c $
 * $Revision: 1.4 $
 * $Author: mahk $
 * $Date: 1994/04/05 00:19:46 $
 *
 * Cursor drawing routines called from the interrupt handler.
 *
 * $Log: curdrw.c $
 * Revision 1.4  1994/04/05  00:19:46  mahk
 * Added hflipped cursors.  Wacky.
 *
 * Revision 1.3  1994/02/07  15:58:02  mahk
 * now we use clipped get_bitmap for saveunder.
 *
 * Revision 1.2  1994/02/07  16:14:09  mahk
 * Changed the canvas to be more renderer/compatible.
 *
 * Revision 1.1  1993/12/16  07:45:47  kaboom
 * Initial revision
 *
 */

#include <stdlib.h>

#include "lg.h"
#include "2d.h"
#include "cursors.h"
#include "curdat.h"

//-----------------------------------------------------------
void cursor_draw_callback(ss_mouse_event *e, void *data) {
    LGPoint pos;

    if (MouseLock > 0)
        return;
    MouseLock++;
    pos.x = e->x;
    pos.y = e->y;
    if (CurrentCursor != NULL) {
        pos.x -= CurrentCursor->hotspot.x;
        pos.y -= CurrentCursor->hotspot.y;
    }
    if (LastCursor != NULL) {
        if (abs(LastCursorPos.x - pos.x) + abs(LastCursorPos.y - pos.y) <= CursorMoveTolerance)
            goto out;
        LastCursor->func(CURSOR_UNDRAW, LastCursorRegion, LastCursor, LastCursorPos);
    }
    if (CurrentCursor != NULL) {
        LGRect cr;
        cr.ul = pos;
        cr.lr.x = pos.x + CurrentCursor->w;
        cr.lr.y = pos.y + CurrentCursor->h;
        if (RECT_TEST_SECT(&HideRect[curhiderect], &cr)) {
            goto out;
        }
        CurrentCursor->func(CURSOR_DRAW, CursorRegion, CurrentCursor, pos);
        LastCursor = CurrentCursor;
        LastCursorPos = pos;
        LastCursorRegion = CursorRegion;
    }
out:
    MouseLock--;
}

// ---------------------
// BITMAP CURSOR SUPPORT
// ---------------------

static grs_canvas *old_canvas = NULL;

//-----------------------------------------------------------
void bitmap_cursor_drawfunc(int cmd, LGRegion *r, LGCursor *c, LGPoint pos) {
    grs_bitmap *bm = (grs_bitmap *)(c->state);

    // set up screen canvas
    old_canvas = grd_canvas;
    gr_set_canvas(CursorCanvas);
    switch (cmd) {
    case CURSOR_UNDRAW:
        gr_bitmap(&SaveUnder.bm, pos.x, pos.y);
        break;

    case CURSOR_DRAW:
        // Get saveunder
        gr_init_bitmap(&SaveUnder.bm, SaveUnder.bm.bits, BMT_FLAT8, 0, bm->w, bm->h);
        gr_get_bitmap(&SaveUnder.bm, pos.x, pos.y);
        // Blit over the save under
        gr_bitmap(bm, pos.x, pos.y);
        break;

    case CURSOR_DRAW_HFLIP:
        pos.x -= bm->w - 1;
        // Get saveunder
        gr_init_bitmap(&SaveUnder.bm, SaveUnder.bm.bits, BMT_FLAT8, 0, bm->w, bm->h);
        gr_get_bitmap(&SaveUnder.bm, pos.x, pos.y);
        // Blit over the save under
        gr_bitmap(bm, pos.x, pos.y);
        break;

    default: {
        ERROR("%s: unknown cursor cmd draw: %d!", __FUNCTION__, cmd);
    }

    }
    gr_set_canvas(old_canvas);
}
