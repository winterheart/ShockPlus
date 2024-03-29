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
 * $Source: r:/prj/lib/src/ui/RCS/cursors.c $
 * $Revision: 1.24 $
 * $Author: mahk $
 * $Date: 1994/08/24 08:55:41 $
 *
 * $Log: cursors.c $
 * Revision 1.24  1994/08/24  08:55:41  mahk
 * Cursor stacks and invisible regions.
 *
 * Revision 1.23  1994/08/22  04:19:08  mahk
 * Made real anal cursor_stack spew.
 *
 * Revision 1.22  1994/02/10  08:15:25  mahk
 * Actually caused uiSetDefaultSlabCursor or whatever to actually work as specified instead
 * of obliterating the top of the cursor stack.
 *
 * Revision 1.21  1994/02/07  16:14:43  mahk
 * Changed the canvas to be more renderer/compatible.
 *
 * Revision 1.20  1994/02/06  06:05:03  mahk
 * Fixed stupid uiPopSlabCursor bug.
 *
 * Revision 1.19  1994/01/19  20:52:11  mahk
 * Fixed black cursor bug yay.
 *
 * Revision 1.18  1994/01/16  04:37:40  mahk
 * Fixed hide/show logic
 *
 * Revision 1.17  1993/12/16  07:44:07  kaboom
 * Moved code actually called from interrupt handler to another
 * file and made relevant statics public.
 *
 * Revision 1.16  1993/10/20  05:13:00  mahk
 * No longer do we push a canvas in an interrupt.
 *
 * Revision 1.15  1993/10/11  20:26:32  dc
 * Angle is fun, fun fun fun
 *
 * Revision 1.14  1993/09/09  19:20:10  mahk
 * Fixed heap trashage.
 *
 * Revision 1.13  1993/08/16  18:24:28  xemu
 * fixed some spew
 *
 * Revision 1.12  1993/08/02  14:21:10  mahk
 * Save under now grows whenever a bitmap cursor is made
 *
 * Revision 1.11  1993/06/08  23:59:36  mahk
 * Cursors now clip.
 *
 * Revision 1.10  1993/05/27  12:49:01  mahk
 * Lock out reentrance in interrupt mouse drawing.
 * Mouse rectangle stack grows in size if necessary.
 *
 * Revision 1.9  1993/05/26  16:33:55  mahk
 * Added REAL mouse tolerance.
 *
 * Revision 1.8  1993/05/26  03:21:54  mahk
 * Added way-cool rectangle protection for mousehide, and pixel move tolerance for cursor
 * draw.
 *
 * Revision 1.7  1993/05/25  19:55:14  mahk
 * Fixed mousehide and show to not leave droppings.
 *
 * Revision 1.6  1993/04/28  15:59:23  mahk
 * conversion to libdbg
 *
 * Revision 1.5  1993/04/28  14:39:52  mahk
 * Preparing for second exodus
 *
 * Revision 1.4  1993/04/13  23:18:35  mahk
 * Added lots of debugging spews.
 *
 * Revision 1.3  1993/04/08  17:52:07  mahk
 * The interrupt handler callback, now, in fact, no longer draws the
 * mouse cursor when it is hidden.  Go Figure.
 *
 * Revision 1.2  1993/04/05  23:36:01  unknown
 * Added hide/show and slab support.
 *
 * Revision 1.1  1993/03/31  23:17:24  mahk
 * Initial revision
 */

#include <stdlib.h>
#include <string.h>

#include "lg.h"
#include "2d.h"
#include "curdrw.h"
#include "cursors.h"
#include "curtyp.h"
#include "slab.h"
#include "vmouse.h"

#define CURSOR_STACKSIZE 5
#define STARTING_SAVEUNDER_WD 16
#define STARTING_SAVEUNDER_HT 16

// -------------------
// DEFINES AND GLOBALS
// -------------------

// Global: the saveunder for bitmap cursors
struct cursor_saveunder SaveUnder;

#define RootCursorRegion (uiCurrentSlab->creg)

// The region currently occupied by the cursor, and the current cursor to be drawn

LGRegion *CursorRegion = NULL;
LGCursor *CurrentCursor = NULL;

// The last cursor region, for when we undraw.

LGPoint LastCursorPos;
LGRegion *LastCursorRegion = NULL;
LGCursor *LastCursor = NULL;

// A semaphore which tells the mouse interrupt handler that the mouse is hidden
int MouseLock = 0;

// A protected rectangle stack for mouse hide/show
#define INITIAL_RECT_STACK 5
LGRect *HideRect;
int numhiderects = INITIAL_RECT_STACK;
int curhiderect = 0;

// The canvas used by cursors
grs_canvas DefaultCursorCanvas;
grs_canvas *CursorCanvas = &DefaultCursorCanvas;

// Number of pixels to move before interrupt handler draws
int CursorMoveTolerance = 0;

// ------------------
// INTERNAL PROTOTYPES
// ------------------
typedef struct cursor_callback_state {
    LGCursor **out;
    LGRegion **reg;
} cstate;

// ------------------
// INTERNAL FUNCTIONS
// ------------------

// KLC - now just allocates a bitmap at the beginning and never actually grows it.
static errtype grow_save_under(short x, short y) {
    int sz = x * y;
    if (SaveUnder.mapsize >= sz)
        return ERR_NOEFFECT;

    // Clear out old SaveUnder
    if (SaveUnder.mapsize > 0) {
        free(SaveUnder.bm.bits);
    }

    // Grow bigger than we actually need so that this doesn't happen all the time
    int newsize = sz * 2;
    SaveUnder.bm.bits = (uchar *)malloc(newsize);
    SaveUnder.mapsize = newsize;
    return OK;
}

uchar cursor_get_callback(LGRegion *reg, LGRect *rect, void *vp) {
    cstate *s = (cstate *)vp;
    cursor_stack *cs = (cursor_stack *)(reg->cursors);

    if (cs == NULL)
        *(s->out) = NULL;
    else {
        *(s->out) = cs->stack[cs->fullness - 1];
        *(s->reg) = reg;
    }
    if (*(s->out) == NULL && uiCurrentSlab != NULL) {
        *(s->out) = uiCurrentSlab->cstack.stack[0];
    }
    return *(s->out) != NULL;
}

errtype uiMakeCursorStack(cursor_stack *res) {
    if (res == NULL)
        return ERR_NULL;
    res->size = CURSOR_STACKSIZE;
    res->stack = (LGCursor **)malloc(sizeof(LGCursor *) * res->size);
    if (res->stack == NULL) {
        free(res);
        return ERR_NOMEM;
    }
    res->fullness = 1;
    res->stack[0] = NULL;
    return OK;
}

errtype uiDestroyCursorStack(cursor_stack *cstack) {
    if (cstack == NULL)
        return ERR_NULL;
    if (cstack->size == 0)
        return ERR_NOEFFECT;
    cstack->size = 0;
    free(cstack->stack);
    return OK;
}

errtype uiSetRegionCursorStack(LGRegion *r, uiCursorStack *cs) {
    if (r == NULL)
        return ERR_NULL;
    r->cursors = cs;
    return OK;
}

errtype uiGetSlabCursorStack(uiSlab *slab, uiCursorStack **cs) {
    if (slab == NULL)
        return ERR_NULL;
    *cs = &slab->cstack;
    return OK;
}

errtype uiSetDefaultCursor(uiCursorStack *cs, LGCursor *c) {
    if (cs == NULL)
        return ERR_NULL;
    cs->stack[0] = c;
    return OK;
}

errtype uiGetDefaultCursor(uiCursorStack *cs, LGCursor **c) {
    if (cs == NULL)
        return ERR_NULL;
    *c = cs->stack[0];
    return OK;
}

errtype uiPushCursor(cursor_stack *cs, LGCursor *c) {
    if (cs == NULL)
        return ERR_NULL;
    if (cs->fullness >= cs->size) {
        LGCursor **tmp = (LGCursor **)malloc(cs->size * 2 * sizeof(LGCursor *));
        if (tmp == NULL)
            return ERR_NOMEM;
        memcpy(tmp, cs->stack, cs->size * sizeof(LGCursor *));
        free(cs->stack);
        cs->stack = tmp;
        cs->size *= 2;
    }
    cs->stack[cs->fullness++] = c;
    return OK;
}

errtype uiPopCursor(uiCursorStack *cs) {
    if (cs == NULL)
        return ERR_NULL;
    if (cs->fullness <= 1)
        return ERR_DUNDERFLOW;
    cs->fullness--;
    return OK;
}

errtype uiGetTopCursor(uiCursorStack *cs, LGCursor **c) {
    if (cs == NULL)
        return ERR_NULL;
    if (cs->fullness <= 1)
        return ERR_DUNDERFLOW;
    *c = cs->stack[cs->fullness - 1];
    return OK;
}

errtype uiPushCursorOnce(uiCursorStack *cs, LGCursor *c) {
    LGCursor *top = NULL;
    errtype err = uiGetTopCursor(cs, &top);
    if (err == ERR_DUNDERFLOW)
        top = NULL;
    else if (err != OK)
        return err;
    if (top != c)
        err = uiPushCursor(cs, c);
    return err;
}

// I wish I had time to implement a non-boneheaded recursive version.
errtype uiPopCursorEvery(uiCursorStack *cs, LGCursor *c) {
    LGCursor *top = NULL;
    errtype err = uiGetTopCursor(cs, &top);
    if (err == ERR_DUNDERFLOW)
        return OK;
    else if (err != OK)
        return err;
    uiPopCursor(cs);
    err = uiPopCursorEvery(cs, c);
    if (top != c) {
        errtype newerr = uiPushCursor(cs, top);
        if (newerr != OK)
            return newerr;
    }
    return err;
}

errtype uiGetRegionCursorStack(LGRegion *r, cursor_stack **cs) {
    cursor_stack *res = (cursor_stack *)(r->cursors);
    if (res == NULL) {
        errtype err;
        r->cursors = res = (cursor_stack *)malloc(sizeof(cursor_stack));
        if (res == NULL)
            return ERR_NOMEM;
        err = uiMakeCursorStack(res);
        if (err != OK)
            return err;
    }
    *cs = res;
    return OK;
}

// --------------------
// UI Toolkit internals
// --------------------

static int uiCursorCallbackId;

errtype ui_init_cursor_stack(uiSlab *slab, LGCursor *default_cursor) {
    errtype err = uiMakeCursorStack(&slab->cstack);
    if (err != OK)
        return err;
    slab->cstack.stack[0] = default_cursor;
    return OK;
}


errtype ui_init_cursors(void) {
    grow_save_under(STARTING_SAVEUNDER_WD, STARTING_SAVEUNDER_HT);
    // KLC - just initalize it to a sizeable bitmap, and leave it that way.
    // SaveUnder.bm.bits = (uchar *)malloc(6144);
    // SaveUnder.mapsize = 6144;

    LastCursor = NULL;
    MouseLock = 0;

    gr_init_sub_canvas(grd_screen_canvas, &DefaultCursorCanvas, 0, 0, grd_cap->w, grd_cap->h);
    gr_cset_cliprect(&DefaultCursorCanvas, 0, 0, grd_cap->w, grd_cap->h);
    errtype err = mouse_set_callback(cursor_draw_callback, NULL, &uiCursorCallbackId);
    if (err != OK)
        return err;
    HideRect = (LGRect *)malloc(sizeof(LGRect) * INITIAL_RECT_STACK);
    HideRect[0].ul.x = -32768;
    HideRect[0].ul.y = -32768;
    HideRect[0].lr = HideRect[0].ul;
    return OK;
}

errtype uiUpdateScreenSize(LGPoint size) {
    short w = size.x;
    short h = size.y;
    if (size.x == UI_DETECT_SCREEN_SIZE.x)
        w = grd_screen_canvas->bm.w;
    if (size.y == UI_DETECT_SCREEN_SIZE.y)
        h = grd_screen_canvas->bm.h;

    gr_init_sub_canvas(grd_screen_canvas, &DefaultCursorCanvas, 0, 0, w, h);
    gr_cset_cliprect(&DefaultCursorCanvas, 0, 0, w, h);
    // mouse_set_screensize(w,h);
    // mouse_constrain_xy(0,0,w,h);
    return (OK);
}

errtype ui_shutdown_cursors(void) {
    free(SaveUnder.bm.bits);
    return mouse_unset_callback(uiCursorCallbackId);
}

uchar ui_set_current_cursor(LGPoint pos) {
    cstate s;
    uchar result = FALSE;

    ui_mouse_do_conversion(&(pos.x), &(pos.y), TRUE);
    if (uiCurrentSlab == NULL) {
        result = FALSE;
        goto out;
    }
    if (uiCurrentSlab->cstack.fullness > 1) {
        CurrentCursor = uiCurrentSlab->cstack.stack[uiCurrentSlab->cstack.fullness - 1];
        CursorRegion = (uiCurrentSlab->creg);
        result = CurrentCursor != NULL;
        goto out;
    }
    if (RootCursorRegion == NULL) {
        result = FALSE;
        goto out;
    }
    s.out = &CurrentCursor;
    s.reg = &CursorRegion;

    result = region_traverse_point(RootCursorRegion, pos, cursor_get_callback, TOP_TO_BOTTOM, &s);
out:
    return result;
}

void ui_update_cursor(LGPoint pos) {
    uchar show = ui_set_current_cursor(pos);
    // ui_mouse_do_conversion(&(pos.x),&(pos.y),FALSE);
    if (show && LastCursor != NULL && !PointsEqual(pos, LastCursorPos)) {
        MouseLock++;
        LastCursor->func(CURSOR_UNDRAW, LastCursorRegion, LastCursor, LastCursorPos);
        LastCursorPos = pos;
        LastCursorPos.x -= CurrentCursor->hotspot.x;
        LastCursorPos.y -= CurrentCursor->hotspot.y;
        CurrentCursor->func(CURSOR_DRAW, CursorRegion, CurrentCursor, LastCursorPos);
        LastCursor = CurrentCursor;
        LastCursorRegion = CursorRegion;
        MouseLock--;
    }
}

// -------------
// API FUNCTIONS
// -------------

errtype uiSetCursor(void) {
    LGPoint pos;
    uchar show = MouseLock == 0;
    errtype retval = OK;
    if (!ui_set_current_cursor(pos)) {
        retval = ERR_NULL;
    }
    if (MouseLock > 0) // KLC - added to keep MouseLock from going negative.
        MouseLock--;
    if (show && CurrentCursor != LastCursor) {
        uiShowMouse(NULL);
    }
    return (retval);
}

errtype uiSetRegionDefaultCursor(LGRegion *r, LGCursor *c) {
    cursor_stack *cs;
    errtype err = uiGetRegionCursorStack(r, &cs);
    if (err != OK)
        return err;
    cs->stack[0] = c;
    uiSetCursor();
    return OK;
}

errtype uiPushRegionCursor(LGRegion *r, LGCursor *c) {
    cursor_stack *cs;
    errtype err = uiGetRegionCursorStack(r, &cs);
    if (err != OK)
        return err;
    err = uiPushCursor(cs, c);
    if (err != OK)
        return err;
    return uiSetCursor();
}

errtype uiPopRegionCursor(LGRegion *r) {
    cursor_stack *cs;
    if (r == NULL)
        return ERR_NULL;
    cs = (cursor_stack *)(r->cursors);
    if (cs == NULL)
        return ERR_DUNDERFLOW;
    else {
        if (cs->fullness <= 1)
            return ERR_DUNDERFLOW;
        cs->fullness--;
        uiSetCursor();
    }
    return OK;
}

errtype uiGetRegionCursor(LGRegion *r, LGCursor **c) {
    cursor_stack *cs;
    if (r == NULL)
        return ERR_NULL;
    cs = (cursor_stack *)(r->cursors);
    if (cs == NULL) {
        *c = NULL;
    } else {
        *c = cs->stack[cs->fullness - 1];
    }
    return OK;
}

errtype uiShutdownRegionCursors(LGRegion *r) {
    cursor_stack *cs = (cursor_stack *)(r->cursors);
    if (cs == NULL)
        return ERR_NOEFFECT;
    free(cs->stack);
    free(cs);
    r->cursors = NULL;
    uiSetCursor();
    return OK;
}

errtype uiSetSlabDefaultCursor(uiSlab *slab, LGCursor *c) {
    if (slab == NULL)
        return ERR_NULL;
    slab->cstack.stack[0] = c;
    uiSetCursor();
    return OK;
}

errtype uiSetGlobalDefaultCursor(LGCursor *c) { return uiSetSlabDefaultCursor(uiCurrentSlab, c); }

errtype uiPushSlabCursor(uiSlab *slab, LGCursor *c) {
    errtype err;
    if (slab == NULL)
        return ERR_NULL;
    err = uiPushCursor(&slab->cstack, c);
    uiSetCursor();
    return err;
}

errtype uiPushGlobalCursor(LGCursor *c) { return uiPushSlabCursor(uiCurrentSlab, c); }

errtype uiPopSlabCursor(uiSlab *slab) {
    if (slab == NULL)
        return ERR_NULL;
    if (slab->cstack.fullness <= 1)
        return ERR_DUNDERFLOW;
    slab->cstack.fullness--;
    uiSetCursor();
    return OK;
}

errtype uiPopGlobalCursor(void) { return uiPopSlabCursor(uiCurrentSlab); }

errtype uiGetSlabCursor(uiSlab *slab, LGCursor **c) {
    if (slab == NULL)
        return ERR_NULL;
    *c = slab->cstack.stack[slab->cstack.fullness - 1];
    uiSetCursor();
    return OK;
}

errtype uiGetGlobalCursor(LGCursor **c) { return uiGetSlabCursor(uiCurrentSlab, c); }

errtype uiHideMouse(LGRect *r) {
    LGRect mr;
    uchar hide = r == NULL || LastCursor == NULL;
    MouseLock++; // hey, don't move the mouse while we're doing this.
    if (!hide) {
        mr.ul = LastCursorPos;
        ui_mouse_do_conversion(&(mr.ul.x), &(mr.ul.y), TRUE);
        if (LastCursor != NULL) {
            mr.lr.x = LastCursorPos.x + LastCursor->w;
            mr.lr.y = LastCursorPos.y + LastCursor->h;
            ui_mouse_do_conversion(&mr.lr.x, &mr.lr.y, TRUE);
        } else
            mr.lr = mr.ul;
        curhiderect++;
        if (curhiderect >= numhiderects) {
            LGRect *tmp = HideRect;
            HideRect = (LGRect *)malloc(numhiderects * 2 * sizeof(LGRect));
            memcpy(HideRect, tmp, numhiderects * sizeof(LGRect));
            numhiderects *= 2;
            free(tmp);
        }
        if (curhiderect == 1)
            HideRect[curhiderect] = *r;
        else
            RECT_UNION(&HideRect[curhiderect - 1], r, &HideRect[curhiderect]);
        hide = RECT_TEST_SECT(&HideRect[curhiderect], &mr);
    }
    // Undraw the mouse.
    if (hide) {
        if (LastCursor != NULL) {
            LastCursor->func(CURSOR_UNDRAW, LastCursorRegion, LastCursor, LastCursorPos);
        }
        LastCursor = NULL;
    }

//#define FREEZE_ON_HIDE
#ifndef FREEZE_ON_HIDE
    else {
        MouseLock--;
        return ERR_NOEFFECT;
    }
#endif
    return OK;
}

errtype uiShowMouse(LGRect *r) {
    errtype ret;
    LGRect mr;
    uchar show = LastCursor == NULL || r == NULL;
    MouseLock++;
    if (!show) {
        mr.ul = LastCursorPos;
        ui_mouse_do_conversion(&(mr.ul.x), &(mr.ul.y), TRUE);
        if (LastCursor != NULL) {
            mr.lr.x = LastCursorPos.x + LastCursor->w;
            mr.lr.y = LastCursorPos.y + LastCursor->h;
            ui_mouse_do_conversion(&mr.lr.x, &mr.lr.y, TRUE);
        } else
            mr.lr = mr.ul;
        show = RECT_TEST_SECT(r, &mr);
    }
    if (show) {
        if (MouseLock <= 2) {
            MouseLock = 2;
            if (LastCursor != NULL) {
                LastCursor->func(CURSOR_UNDRAW, LastCursorRegion, LastCursor, LastCursorPos);
            }
            mouse_get_xy(&LastCursorPos.x, &LastCursorPos.y);
            if (ui_set_current_cursor(LastCursorPos)) {
                LastCursorPos.x -= CurrentCursor->hotspot.x;
                LastCursorPos.y -= CurrentCursor->hotspot.y;
                CurrentCursor->func(CURSOR_DRAW, CursorRegion, CurrentCursor, LastCursorPos);
                LastCursor = CurrentCursor;
                LastCursorRegion = CursorRegion;
            } else
                LastCursor = NULL;
        }
        MouseLock--;
        ret = OK;
    }
#ifndef FREEZE_ON_HIDE
    else {
        ret = ERR_NOEFFECT;
    }
#else
    else {
        if (MouseLock <= 2)
            MouseLock = 2;
        MouseLock--;
    }
#endif
    if (--curhiderect < 0)
        curhiderect = 0;
    MouseLock--;

    // KLC - I have no idea what MouseLock is doing.  I'll just set it to zero - shown - duh!
    MouseLock = 0;

    return ret;
}

errtype uiMakeBitmapCursor(LGCursor *c, grs_bitmap *bm, LGPoint hotspot) {
    if (c == NULL) {
        WARN("%s: Tried to make a null cursor!", __FUNCTION__ );
        return ERR_NOEFFECT;
    }

    grow_save_under(bm->w, bm->h);
    c->func = bitmap_cursor_drawfunc;
    c->state = bm;
    c->hotspot = hotspot;
    c->w = bm->w;
    c->h = bm->h;
    return OK;
}
