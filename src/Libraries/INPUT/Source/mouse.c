/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.
Copyright (C) 2019 Shockolate Project

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
 * $Source: n:/project/lib/src/input/RCS/mouse.c $
 * $Revision: 1.15 $
 * $Author: mahk $
 * $Date: 1994/06/21 06:16:42 $
 *
 * Mouse handler adapted from Rex Bradford's Freefall mouse code.
 *
 * This file is part of the input library.
 */

//    This file only vaguely resembles the freefall mouse code from
//    which it descended.  It supports some very-low-level input routines
//    for the mouse.  It provides an interrupt-driven event queue, polling,
//    and callbacks from the interrupt handler.

//	---------------------------------------------------------
// 6/21/94 ML Added mouse velocity support in mousevel.h so that we can emulate the mouse
// using other devices.
//	---------------------------------------------------------
// For the Mac version I use a TimeManager task to poll the mouse for mouse
// movement callback routines.  Mouse click events will be handled throught the normal
// Macintosh event queue.  Most of the stuff in this file will go away.
// ¥¥¥Note:  The mouse position will always be returned in *local* coordinates,
// that is, local to the main game window.

#include "mouse.h"
#include "sdl_events.h"
#include "tickcount.h"

typedef struct _mouse_state {
    short x, y;
    short butts;
} mouse_state;

short mouseInstantButts;

#define NUM_MOUSE_CALLBACKS 16
mouse_callfunc mouseCall[NUM_MOUSE_CALLBACKS];
void *mouseCallData[NUM_MOUSE_CALLBACKS];
short mouseCalls = 0; // current number of mouse calls.

// latest mouse state as input for MousePollProc() in mouse.c
ss_mouse_event latestMouseEvent;

//----------------
// Internal Prototypes
//----------------
static void ReadMouseState(mouse_state *pMouseState);

/**
 * terminates mouse handler.
 * @deprecated does nothing
 * @return OK code
 */
errtype mouse_shutdown(void) { return OK; }

//	---------------------------------------------------------
//	mouse_init() initializes mouse handler.  It does the following:
//
//	1. Microsoft mouse driver initialized
//	2. Customizes mouse driver & handler based on display mode
//	3. Initializes mouse handler state variables
//	---------------------------------------------------------
//  For Mac version: ignore sizes (mouse is already set up).
errtype mouse_init(short mone, short mtwo) {
    mouse_state mstate;
    // Initialize mouse state variables
    sdl_mouse_init();

    mouseCalls = 0;

    ReadMouseState(&mstate);
    mouseInstantButts = mstate.butts;
    return OK;
}

// ------------------------------------------------------
//
// mouse_set_callback() registers a callback with the interrupt handler
// f = func to be called back.
// data = data to be given to the func when called
// *id = set to a unique id of the callback.

errtype mouse_set_callback(mouse_callfunc f, void *data, int *id) {
    // Spew(DSRC_MOUSE_SetCallback,("entering mouse_set_callback(%x,%x,%x)\n",f,data,id));
    for (*id = 0; *id < mouseCalls; ++*id)
        if (mouseCall[*id] == NULL)
            break;
    if (*id == NUM_MOUSE_CALLBACKS) {
        // Spew(DSRC_MOUSE_SetCallback,("mouse_set_callback(): Table Overflow.\n"));
        return ERR_DOVERFLOW;
    }
    if (*id == mouseCalls)
        mouseCalls++;
    // Spew(DSRC_MOUSE_SetCallback,("mouse_set_callback(): *id = %d, mouseCalls = %d\n",*id,mouseCalls));
    mouseCall[*id] = f;
    mouseCallData[*id] = data;
    return OK;
}

// -------------------------------------------------------
//
// mouse_unset_callback() un-registers a callback function
// id = unique id of function to unset

errtype mouse_unset_callback(int id) {
    // Spew(DSRC_MOUSE_UnsetCallback,("entering mouse_unset_callback(%d)\n",id));
    if (id >= mouseCalls || id < 0) {
        // Spew(DSRC_MOUSE_UnsetCallback,("mouse_unset_callback(): id out of range \n"));
        return ERR_RANGE;
    }
    mouseCall[id] = NULL;
    while (mouseCalls > 0 && mouseCall[mouseCalls - 1] == NULL)
        mouseCalls--;
    return OK;
}

/**
 * mouse_constrain_xy() defines min/max coords
 * @deprecated does nothing
 * @param xl
 * @param yl
 * @param xh
 * @param yh
 * @return OK code
 */
errtype mouse_constrain_xy(short xl, short yl, short xh, short yh) { return OK; }

// --------------------------------------------------------
// mouse_get_time() returns the current mouse timestamp
//	--------------------------------------------------------
// For Mac version:  Just return TickCount().

uint32_t mouse_get_time(void) { return TickCount(); }

//	--------------------------------------------------------
//	ReadMouseState() reads current state of mouse.
//
//		pMouseState = ptr to mouse state struct, filled in by routine
//	--------------------------------------------------------
// For Mac version:  Use Mac routines to get mouse position and state.

static void ReadMouseState(mouse_state *pMouseState) {
    int mouse_x;
    int mouse_y;

    uint mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    pMouseState->x = mouse_x;
    pMouseState->y = mouse_y;
    pMouseState->butts = 0;

    if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        pMouseState->butts = 1;
    }

    if (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        pMouseState->butts = 2;
    }
}
