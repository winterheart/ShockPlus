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
//		Mouse.H		Mouse library header file
//		MAHK Leblanc 2/19/93
/*
 * $Source: n:/project/lib/src/input/RCS/mouse.h $
 * $Revision: 1.11 $
 * $Author: unknown $
 * $Date: 1993/09/01 00:19:18 $
 *
 * $Log: mouse.h $
 * Revision 1.11  1993/09/01  00:19:18  unknown
 * Changed left-handedness api
 *
 * Revision 1.10  1993/08/29  03:12:35  mahk
 * Added mousemask and lefty support.
 *
 * Revision 1.9  1993/08/27  14:05:06  mahk
 * Added shift factors
 *
 * Revision 1.8  1993/07/28  18:15:58  jak
 * Added mouse_extremes() function
 *
 * Revision 1.7  1993/06/28  02:04:59  mahk
 * Bug fixes for the new regime
 *
 * Revision 1.6  1993/06/27  22:17:32  mahk
 * Added timestamps and button state to the mouse event structure.
 *
 * Revision 1.5  1993/05/04  14:34:27  mahk
 * mouse_init no longer takes a screen mode argument.
 *
 * Revision 1.4  1993/04/14  12:08:46  mahk
 * Hey, I got my mouse ups and downs backwards.
 *
 * Revision 1.3  1993/03/19  18:46:57  mahk
 * Added RCS header
 *
 *
 */

#ifndef MOUSE_H
#define MOUSE_H

#include <SDL_events.h>

#include "lg.h"
#include "lg_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mouse_event {
    short x;            // X-position
    short y;            // Y-position
    uint16_t type;      // Event mask, bits defined below
    uint32_t timestamp; // Event timestamp
    uchar buttons;      // Buttons pressed
    uchar modifiers;    // Added for Mac version
    SDL_Event event;    // Oh yeah, train wreck goes full steam
    char pad[4];        // pad to sixteen bytes
} ss_mouse_event;

#define MOUSE_MOTION 1u // Event mask bits
#define MOUSE_LDOWN 2u
#define MOUSE_LUP 4u
#define MOUSE_RDOWN 8u
#define MOUSE_RUP 16u
#define MOUSE_CDOWN 32u
#define MOUSE_CUP 64u
// bits 7..9 are used for double click events, see UI/Source/event.h
#define MOUSE_WHEELUP (1 << 10)
#define MOUSE_WHEELDN (1 << 11)

extern short mouseInstantButts;

// latest mouse state as input for MousePollProc() in mouse.c
extern ss_mouse_event latestMouseEvent;

// type of mouse interrupt callback func
typedef void (*mouse_callfunc)(ss_mouse_event *e, void *data);

#define NUM_MOUSE_BTNS 3
#define MOUSE_LBUTTON 0
#define MOUSE_RBUTTON 1
#define MOUSE_CBUTTON 2

#define MOUSE_BTN2DOWN(num) (1 << (1 + 2 * (num)))
#define MOUSE_BTN2UP(num) (1 << (2 + 2 * (num)))

// Initialize the mouse, specifying screen size.
errtype mouse_init(short xsize, short ysize);

// shutdown mouse system
errtype mouse_shutdown(void);

// Get the current mouse timestamp
uint32_t mouse_get_time(void);

// Get the mouse position
errtype mouse_get_xy(short *x, short *y);

// Set the mouse position
errtype mouse_put_xy(short x, short y);

// Flush the mouse queue
errtype mouse_flush(void);

// Set up an interrupt callback
errtype mouse_set_callback(mouse_callfunc f, void *data, int *id);

// Remove an interrupt callback
errtype mouse_unset_callback(int id);

// Constrain the mouse coordinates
errtype mouse_constrain_xy(short xl, short yl, short xh, short yh);

#ifdef __cplusplus
}
#endif

#endif // MOUSE_H
