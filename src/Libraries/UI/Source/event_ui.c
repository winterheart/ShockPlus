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
#include <stdlib.h>
#include <string.h>

#include "frcursors.h"
#include "lg.h"
#include "mouse.h"
#include "movekeys.h"
#include "kb.h"
#include "kbcook.h"
#include "array.h"
#include "rect.h"
#include "slab.h"
#include "event_ui.h"
#include "vmouse.h"

// ---------------------
// INTERNAL PROTOTYPES
// ---------------------
uchar region_check_opacity(LGRegion *reg, ulong evmask);
uchar event_dispatch_callback(LGRegion *reg, LGRect *r, void *v);
void ui_set_last_mouse_region(LGRegion *reg, uiEvent *ev);
uchar ui_try_region(LGRegion *reg, LGPoint pos, uiEvent *ev);
uchar ui_traverse_point(LGRegion *reg, LGPoint pos, uiEvent *data);
void ui_flush_mouse_events(ulong timestamp, LGPoint pos);
void ui_poll_keyboard(void);

// ---------------------
// HANDLER CHAIN DEFINES
// ---------------------

typedef struct ui_event_handler {
    uint32_t typemask;  // Which event types does this handle?
    uiHandlerProc proc; // handler proc: called when a specific event is received
    intptr_t state;     // handler-specific state data
    int32_t next;       // used for chaining handlers.
} uiEventHandler;

typedef struct handler_chain {
    Array chain;
    int front;
    uint32_t opacity;
} handler_chain;

#define INITIAL_CHAINSIZE 4
#define INITIAL_FOCUSES 5
#define CHAIN_END (-1)

uint32_t uiGlobalEventMask = ALL_EVENTS;

uint32_t last_mouse_draw_time = 0;

// ----------------------------
// HANDLER CHAIN IMPLEMENTATION
// ----------------------------

errtype uiInstallRegionHandler(LGRegion *r, uint32_t evmask, uiHandlerProc callback, intptr_t state, int *id) {
    int i;
    if (callback == NULL || r == NULL || evmask == 0)
        return ERR_NULL;
    handler_chain *ch = (handler_chain *)r->handler;
    if (ch == NULL) {
        ch = (handler_chain *)malloc(sizeof(handler_chain));
        if (ch == NULL) {
            return ERR_NOMEM;
        }
        array_init(&ch->chain, sizeof(uiEventHandler), INITIAL_CHAINSIZE);
        ch->front = CHAIN_END;
        r->handler = (void *)ch;
        ch->opacity = uiDefaultRegionOpacity;
    }
    errtype err = array_newelem(&ch->chain, &i);
    if (err != OK) {
        return err;
    }
    uiEventHandler *eh = &((uiEventHandler *)(ch->chain.vec))[i];
    eh->next = ch->front;
    eh->typemask = evmask;
    eh->proc = callback;
    eh->state = state;
    ch->front = i;
    ch->opacity &= ~evmask;
    *id = i;
    return OK;
}

errtype uiRemoveRegionHandler(LGRegion *r, int id) {
    errtype err;

    if (r == NULL)
        return ERR_NULL;
    handler_chain *ch = (handler_chain *)r->handler;
    if (ch == NULL || id < 0)
        return ERR_RANGE;
    uiEventHandler *handlers = (uiEventHandler *)(ch->chain.vec);
    if (id == ch->front) {
        int next = handlers[id].next;
        err = array_dropelem(&ch->chain, id);
        if (err != OK)
            return err;
        ch->front = next;
        return OK;
    }
    for (int i = ch->front; handlers[i].next != CHAIN_END; i = handlers[i].next) {
        if (handlers[i].next == id) {
            err = array_dropelem(&ch->chain, id);
            if (err != OK)
                return err;
            handlers[i].next = handlers[id].next;
            return OK;
        }
    }
    return ERR_NOEFFECT;
}

errtype uiSetRegionHandlerMask(LGRegion *r, int id, int evmask) {
    if (r == NULL)
        return ERR_NULL;
    handler_chain *ch = (handler_chain *)r->handler;
    if (ch == NULL || id >= ch->chain.fullness || id < 0)
        return ERR_RANGE;
    uiEventHandler *handlers = (uiEventHandler *)(ch->chain.vec);
    handlers[id].typemask = evmask;
    return OK;
}

// -------
// OPACITY
// -------

uint32_t uiDefaultRegionOpacity = 0;

ulong uiGetRegionOpacity(LGRegion *reg) {
    handler_chain *ch = (handler_chain *)(reg->handler);
    if (ch == NULL) {
        return uiDefaultRegionOpacity;
    } else {
        return ch->opacity;
    }
}

errtype uiSetRegionOpacity(LGRegion *reg, ulong mask) {
    handler_chain *ch = (handler_chain *)(reg->handler);
    if (ch == NULL) {
        ch = (handler_chain *)malloc(sizeof(handler_chain));
        if (ch == NULL) {
            return ERR_NOMEM;
        }
        array_init(&ch->chain, sizeof(uiEventHandler), INITIAL_CHAINSIZE);
        ch->front = CHAIN_END;
        reg->handler = (void *)ch;
        ch->opacity = mask;
    } else {
        ch->opacity = mask;
    }
    return OK;
}

// -------------------
// FOCUS CHAIN DEFINES
// -------------------

typedef struct _focus_link {
    LGRegion *reg;
    ulong evmask;
    int next;
} focus_link;

#define FocusChain (uiCurrentSlab->fchain.chain)
#define CurFocus (uiCurrentSlab->fchain.curfocus)
#define FCHAIN ((focus_link *)(uiCurrentSlab->fchain.chain.vec))

// ----------------
// FOCUS CHAIN CODE
// ----------------

errtype uiGrabSlabFocus(uiSlab *slab, LGRegion *r, ulong evmask) {
    int i;
    focus_link *fchain = (focus_link *)slab->fchain.chain.vec;
    if (r == NULL)
        return ERR_NULL;
    if (evmask == 0)
        return ERR_NOEFFECT;
    errtype err = array_newelem(&slab->fchain.chain, &i);
    if (err != OK)
        return err;
    fchain[i].reg = r;
    fchain[i].evmask = evmask;
    fchain[i].next = slab->fchain.curfocus;
    slab->fchain.curfocus = i;
    return OK;
}

errtype uiGrabFocus(LGRegion *r, ulong evmask) { return uiGrabSlabFocus(uiCurrentSlab, r, evmask); }

errtype uiReleaseSlabFocus(uiSlab *slab, LGRegion *r, ulong evmask) {
    errtype retval = ERR_NOEFFECT;
    focus_link *fchain = (focus_link *)slab->fchain.chain.vec;
    focus_link *l = &fchain[CurFocus];
    if (r == NULL)
        return ERR_NULL;
    if (l->reg == r) {
        ulong tmpmask = l->evmask & evmask;
        l->evmask &= ~evmask;
        evmask &= ~tmpmask;
        if (l->evmask == 0) {
            int tmp = slab->fchain.curfocus;
            slab->fchain.curfocus = l->next;
            array_dropelem(&slab->fchain.chain, tmp);
        }
        if (evmask == 0)
            return OK;
        retval = OK;
    }
    for (; l->next != CHAIN_END; l = &fchain[l->next]) {
        focus_link *thenext = &fchain[l->next];
        if (thenext->reg == r) {
            ulong tmpmask = l->evmask & evmask;
            thenext->evmask &= ~evmask;
            evmask &= ~tmpmask;
            if (thenext->evmask == 0) {
                int tmp = l->next;
                l->next = thenext->next;
                array_dropelem(&slab->fchain.chain, tmp);
            }
            if (evmask == 0)
                return OK;
            retval = OK;
        }
    }
    return retval;
}

errtype uiReleaseFocus(LGRegion *r, ulong evmask) { return uiReleaseSlabFocus(uiCurrentSlab, r, evmask); }

// -----------------------
// POLLING AND DISPATCHING
// -----------------------

#define INITIAL_QUEUE_SIZE 32
#define DEFAULT_DBLCLICKTIME 0
#define DEFAULT_DBLCLICKDELAY 0

ushort uiDoubleClickTime = DEFAULT_DBLCLICKTIME;
ushort uiDoubleClickDelay = DEFAULT_DBLCLICKDELAY;
uchar uiDoubleClicksOn[NUM_MOUSE_BTNS] = {FALSE, FALSE, FALSE};
uchar uiAltDoubleClick = FALSE;
ushort uiDoubleClickTolerance = 5;
static uiEvent last_down_events[NUM_MOUSE_BTNS];
static uiEvent last_up_events[NUM_MOUSE_BTNS];

static struct _eventqueue {
    int in, out;
    int size;
    uiEvent *vec;
} EventQueue;

// TRUE we are opaque to this mask.
uchar region_check_opacity(LGRegion *reg, ulong evmask) { return (evmask & uiGetRegionOpacity(reg)) != 0; }

uchar event_dispatch_callback(LGRegion *reg, LGRect *rect, void *v) {
    uiEvent *ev = (uiEvent *)v;
    handler_chain *ch = (handler_chain *)(reg->handler);
    int i, next;

    if (ch == NULL) {
        return FALSE;
    }

    uiEventHandler *handlers = (uiEventHandler *)(ch->chain.vec);

    if (handlers == NULL) {
        return FALSE;
    }
    for (i = ch->front; i != CHAIN_END; i = next) {
        next = handlers[i].next;
        if ((handlers[i].typemask & ev->type) && (handlers[i].proc)(ev, reg, handlers[i].state)) {
            return TRUE;
        }
    }
    return FALSE;
}

// ui_traverse_point return values:
#define TRAVERSE_HIT 0
#define TRAVERSE_MISS 1
#define TRAVERSE_OPAQUE 2

LGRegion *uiLastMouseRegion[NUM_MOUSE_BTNS];

void ui_set_last_mouse_region(LGRegion *reg, uiEvent *ev) {
    if (ev->type != UI_EVENT_MOUSE) {
        return;
    }
    for (int i = 0; i < NUM_MOUSE_BTNS; i++) {
        if ((ev->mouse_data.action & MOUSE_BTN2DOWN(i)) != 0 || (ev->mouse_data.action & UI_MOUSE_BTN2DOUBLE(i))) {
            uiLastMouseRegion[i] = reg;
        }
        if (ev->mouse_data.action & MOUSE_BTN2UP(i)) {
            uiLastMouseRegion[i] = NULL;
        }
    }
}

uchar ui_try_region(LGRegion *reg, LGPoint pos, uiEvent *ev) {
    LGRect cbr;
    uchar retval = TRAVERSE_MISS;

    cbr.ul = pos;
    cbr.lr = pos;
    if (region_check_opacity(reg, ev->type)) {
        retval = TRAVERSE_OPAQUE;
    } else if (event_dispatch_callback(reg, &cbr, ev)) {
        retval = TRAVERSE_HIT;
    } else {
        return retval;
    }
    ui_set_last_mouse_region(reg, ev);
    return retval;
}

uchar ui_traverse_point(LGRegion *reg, LGPoint pos, uiEvent *data) {
    uchar retval = TRAVERSE_MISS;
    LGRegion *child;

    LGPoint rel = pos;
    rel.x -= reg->abs_x;
    rel.y -= reg->abs_y;

    if ((reg->status_flags & INVISIBLE_FLAG) != 0) {
        return retval;
    }

    if (reg->event_order) {
        retval = ui_try_region(reg, pos, data);
        if (retval != TRAVERSE_MISS) {
            return retval;
        }
    }
    for (child = reg->sub_region; child != NULL; child = child->next_region)
        if (RECT_TEST_PT(child->r, rel)) {
            retval = ui_traverse_point(child, pos, data);
            if (retval != TRAVERSE_MISS) {
                return retval;
            }
            break;
        }
    if (!reg->event_order) {
        retval = ui_try_region(reg, pos, data);
        if (retval != TRAVERSE_MISS) {
            return retval;
        }
    }
    return TRAVERSE_MISS;
}

uchar uiDispatchEventToRegion(uiEvent *ev, LGRegion *reg) {
    uiEvent nev = *ev;

    ui_mouse_do_conversion(&(nev.pos.x), &(nev.pos.y), TRUE);
    LGPoint pos = nev.pos;
    pos.x += reg->r->ul.x - reg->abs_x;
    pos.y += reg->r->ul.y - reg->abs_y;

    if (!RECT_TEST_PT(reg->r, pos)) {
        LGRect r;
        r.ul = nev.pos;
        r.lr.x = nev.pos.x + 1;
        r.lr.y = nev.pos.y + 1;
        return event_dispatch_callback(reg, &r, &nev);
    }
    return ui_traverse_point(reg, nev.pos, &nev) == TRAVERSE_HIT;
}

uchar uiDispatchEvent(uiEvent *ev) {
    if (!(ev->type & uiGlobalEventMask)) {
        return FALSE;
    }
    for (int i = CurFocus; i != CHAIN_END; i = FCHAIN[i].next) {
        if (FCHAIN[i].evmask & ev->type) {
            if (uiDispatchEventToRegion(ev, FCHAIN[i].reg)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

#define MOUSE_EVENT_FLUSHED UI_EVENT_MOUSE_MOVE

void ui_flush_mouse_events(ulong timestamp, LGPoint pos) {
    for (int i = 0; i < NUM_MOUSE_BTNS; i++) {
        if (uiDoubleClicksOn[i] && last_down_events[i].type != UI_EVENT_NULL) {
            int crit = uiDoubleClickDelay * 5;
            ulong timediff = timestamp - last_down_events[i].mouse_data.tstamp;
            LGPoint downpos = last_down_events[i].pos;
            uchar out =
                (abs(pos.x - downpos.x) > uiDoubleClickTolerance || abs(pos.y - downpos.y) > uiDoubleClickTolerance);

            // OK, if we've waited DoubleClickDelay after a down event, send it out.
            if (out || timediff >= crit) {
                uiEvent ev;
                if (last_down_events[i].type != MOUSE_EVENT_FLUSHED) {
                    ev = last_down_events[i];
                    last_down_events[i].type = MOUSE_EVENT_FLUSHED;
                    uiDispatchEvent(&ev);
                }
                if (last_up_events[i].type != MOUSE_EVENT_FLUSHED) {
                    ev = last_up_events[i];
                    last_up_events[i].type = MOUSE_EVENT_FLUSHED;
                    uiDispatchEvent(&ev);
                }
            }

            // This is where we do our flushing
            if (last_up_events[i].type != UI_EVENT_NULL) {
                crit = uiDoubleClickTime;
                timediff = timestamp - last_up_events[i].mouse_data.tstamp;
                if (out || timediff >= crit) {
                    last_down_events[i].type = UI_EVENT_NULL;
                    last_up_events[i].type = UI_EVENT_NULL;
                }
            }
        }
    }
}

void ui_dispatch_mouse_event(uiEvent *mout) {
    uchar eaten = FALSE;

    bool altDown = (SDL_GetModState() & KMOD_ALT) != 0;

    // ui_mouse_do_conversion(&(mout->pos.x),&(mout->pos.y),TRUE);
    ui_flush_mouse_events(mout->mouse_data.tstamp, mout->pos);
    for (int i = 0; i < NUM_MOUSE_BTNS; i++) {
        if (!(uiDoubleClicksOn[i]))
            continue;

        if (uiAltDoubleClick && altDown) {
            if (mout->mouse_data.action & MOUSE_BTN2DOWN(i)) {
                mout->mouse_data.action &= ~MOUSE_BTN2DOWN(i);
                mout->mouse_data.action |= UI_MOUSE_BTN2DOUBLE(i);
                continue;
            }
        }
        if (last_down_events[i].type != UI_EVENT_NULL) {
            if (mout->mouse_data.action & MOUSE_BTN2DOWN(i)) {
                // make a double click event.
                mout->mouse_data.action &= ~MOUSE_BTN2DOWN(i);
                mout->mouse_data.action |= UI_MOUSE_BTN2DOUBLE(i);

                last_down_events[i].type = UI_EVENT_NULL;
                last_up_events[i].type = UI_EVENT_NULL;
            }
            if (mout->mouse_data.action & MOUSE_BTN2UP(i)) {
                last_up_events[i] = *mout;
                eaten = TRUE;
            }
        } else if (mout->mouse_data.action & MOUSE_BTN2DOWN(i)) {
            last_down_events[i] = *mout;
            eaten = TRUE;
        }
    }
    if (!eaten) {
        uiDispatchEvent(mout);
    }
}

// ----------------------
// KEYBOARD POLLING SETUP
// ----------------------

/// Polling keys array
uchar *ui_poll_keys = NULL;

errtype uiSetKeyboardPolling(ubyte *codes) {
    ui_poll_keys = codes;
    return OK;
}

/// Convert kb.h mods into SDL_Keymods
static SDL_Keymod inputModToKeyMod(uchar mod) {
    SDL_Keymod ret = KMOD_NONE;
    if (mod & KB_MOD_CTRL)
        ret |= KMOD_CTRL;
    if (mod & KB_MOD_SHIFT)
        ret |= KMOD_SHIFT;
    if (mod & KB_MOD_ALT)
        ret |= KMOD_ALT;
    return ret;
}

static ushort inputModToUImod(uchar mod) {
    ushort ret = 0;
    if (mod & KB_MOD_CTRL)
        ret |= KB_FLAG_CTRL;
    if (mod & KB_MOD_SHIFT)
        ret |= KB_FLAG_SHIFT;
    // TODO: what's 0x04 ? windows key?
    if (mod & KB_MOD_ALT)
        ret |= KB_FLAG_ALT;
    // Note: KB_MOD_PRESSED doesn't matter here

    return ret;
}

/// Keyboard UI_EVENT_KBD_POLL events creator/injector
// KLC - For Mac version, call GetKeys once at the beginning, then check
// the results in the loop.  Fill in the "mods" field (ready for cooking)
// before dispatching an event.
void ui_poll_keyboard(void) {
    // *key is a System Shock/Mac keycode
    for (uchar *key = ui_poll_keys; *key != KBC_NONE; key++) {
        if (sshockKeyStates[*key] != 0) {
            uiEvent ev;
            ev.type = UI_EVENT_KBD_POLL;
            ev.pos.x = 0;
            ev.pos.y = 0;
            ev.poll_key_data.action = KBS_DOWN;
            ev.poll_key_data.scancode = *key;
            ev.poll_key_data.mods = inputModToUImod(sshockKeyStates[*key]);
            // Convert UI_EVENT_KBD_POLL into stubbed SDL_Event
            ev.sdl_data.type = SDL_KEYDOWN;
            ev.sdl_data.key.keysym.scancode = scan_on_motion[*key];
            ev.sdl_data.key.keysym.mod = inputModToKeyMod(sshockKeyStates[*key]);

            uiDispatchEvent(&ev);
        }
    }
}

errtype uiMakeMotionEvent(uiEvent *ev) {
    mouse_get_xy(&ev->pos.x, &ev->pos.y);
    ev->type = UI_EVENT_MOUSE_MOVE; // must get past event mask
    ev->mouse_data.action = MOUSE_MOTION;
    ev->mouse_data.tstamp = mouse_get_time();
    ev->mouse_data.buttons = (ubyte)mouseInstantButts;
    return OK;
}

errtype uiPoll(void) {
    static LGPoint last_mouse = {-1, -1};
    LGPoint mousepos = last_mouse;
    mouse_get_xy(&mousepos.x, &mousepos.y);

    if (ui_poll_keys != NULL && (uiGlobalEventMask & UI_EVENT_KBD_POLL)) {
        ui_poll_keyboard();
    }
    ui_flush_mouse_events(mouse_get_time(), mousepos);

    // CC: Make sure the attack cursor doesn't display forever!
    int diff = mouse_get_time() - last_mouse_draw_time;

    if (!PointsEqual(mousepos, last_mouse) || diff > uiDoubleClickDelay * 5) {
        ui_update_cursor(mousepos);
        last_mouse = mousepos;
        last_mouse_draw_time = mouse_get_time();
    }

    return OK;
}

errtype uiFlush(void) {
    // Flush keyboard and mouse events from queue
    DEBUG("%s: FLUSH input events!", __FUNCTION__);
    SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
    SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEWHEEL);
    return OK;
}

// ---------------------------
// INITIALIZATION AND SHUTDOWN
// ---------------------------

errtype uiInit(uiSlab *slab) {
    uiSetCurrentSlab(slab);
    // initialize the event queue;
    EventQueue.in = EventQueue.out = 0;
    EventQueue.size = INITIAL_QUEUE_SIZE;
    EventQueue.vec = (uiEvent *)malloc(sizeof(uiEvent) * INITIAL_QUEUE_SIZE);
    for (int i = 0; i < NUM_MOUSE_BTNS; i++) {
        last_down_events[i].type = UI_EVENT_NULL;
    }
    // KLC -   AtExit(uiShutdown);
    return OK;
}

void uiShutdown(void) {
    ui_shutdown_cursors();
    mouse_shutdown();
    kb_shutdown();
}

errtype uiShutdownRegionHandlers(LGRegion *r) {
    handler_chain *ch = (handler_chain *)(r->handler);
    if (ch == NULL) {
        return ERR_NOEFFECT;
    }
    errtype err = array_destroy(&ch->chain);
    free(ch);
    return err;
}

errtype ui_init_focus_chain(uiSlab *slab) {
    errtype err = array_init(&slab->fchain.chain, sizeof(focus_link), INITIAL_FOCUSES);
    if (err != OK) {
        return err;
    }
    slab->fchain.curfocus = CHAIN_END;
    return OK;
}
