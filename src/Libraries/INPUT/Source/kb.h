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
#ifndef __KB_H
#define __KB_H
/*
 * $Source: n:/project/lib/src/input/RCS/kbs.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1994/02/12 18:28:21 $
 *
 * Types for keyboard system.
 *
 * This file is part of the input library.
 */

#include <SDL.h>

#ifndef __KBS_H
#define __KBS_H
typedef struct {
    uchar code;         // SDL_Scancode
    uchar state;        // Always SDL_KEYDOWN
    uchar ascii;        // Contains code + modifiers combined code
    uchar modifiers;    // SDL_Keymod
    SDL_Event event;    // Oh yeah, train wreck goes full steam
} kbs_event;
#endif /* !__KBS_H */

/*
 * $Source: n:/project/lib/src/input/RCS/kbdecl.h $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1994/02/12 18:21:29 $
 *
 * Declarations for keyboard library.
 *
 * $Log: kbdecl.h $
 * Revision 1.4  1994/02/12  18:21:29  kaboom
 * Moved event structure.
 *
 * Revision 1.3  1993/04/29  17:19:59  mahk
 * added kb_get_cooked
 *
 * Revision 1.2  1993/04/28  17:01:48  mahk
 * Added kb_flush_bios
 *
 * Revision 1.1  1993/03/10  17:16:41  kaboom
 * Initial revision
 *
 */

extern int kb_startup(void *init_buf);
extern int kb_shutdown(void);
extern void kb_flush(void);

#define KBF_BLOCK (1)
#define KBF_CHAIN (2)
#define KBF_SIGNAL (4)

// FIXME Remove this after migration to SDL_Keycode
#define KBC_SHIFT_PREFIX (0x0e0)
#define KBC_PAUSE_PREFIX (0x0e1)
#define KBC_PAUSE_DOWN (0x0e11d)
#define KBC_PAUSE_UP (0x0e19d)
#define KBC_PRSCR_DOWN (0x02a)
#define KBC_PRSCR_UP (0x0aa)
#define KBC_PAUSE (0x07f)
#define KBC_NONE (0x0ff)

#define KBS_UP (0)
#define KBS_DOWN (1)

// DG: constants for values of kbs_event::modifiers, also used in sshockKeyStates[]
//     (those constants are based on the values that were hardcoded in kb_cook())
#define KB_MOD_CTRL (0x01)
#define KB_MOD_SHIFT (0x04)
#define KB_MOD_ALT (0x08)

// currente state of the keys, based on the SystemShock/Mac Keycodes (sshockKeyStates[keyCode] has the state for that
// key)
extern uchar sshockKeyStates[256];
// this one is only used in sshockKeyStates[], it's set if a button is pressed
// (together with the CTRL/SHIFT/ALT modifiers, if they were pressed as while the key was pressed)
#define KB_MOD_PRESSED (0x10)

#endif /* !__KB_H */
