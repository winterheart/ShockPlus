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
 * $Source: r:/prj/lib/src/input/RCS/kbcook.c $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1994/08/15 16:35:59 $
 *
 * Routines to convert raw scan codes to more useful cooked codes.
 *
 * This file is part of the input library.
 */

#include "lg.h"
#include "kbcook.h"
#include "keydefs.h"

//----------------------------------------------------------------------------
// This cooks kbc codes into ui codes which include ascii stuff.
//----------------------------------------------------------------------------
//  For Mac version, replace the whole thing, because the "cooked" info
//  already exists in the kbs_event record.  So just format the results
//  as expected.
//----------------------------------------------------------------------------
errtype kb_cook(kbs_event ev, ushort *cooked, uchar *results) {
    // On the Mac, since modifiers by themselves don't produce an event,
    // you always have a "cooked" result.

    *results = TRUE;
    *cooked = ev.ascii;

    if (ev.ascii == 0) {
        // FIXME: Why don't the arrow keys translate properly?
        if (ev.code == 125)
            *cooked = KEY_DOWN;
        else if (ev.code == 126)
            *cooked = KEY_UP;
        else if (ev.code == 123)
            *cooked = KEY_LEFT;
        else if (ev.code == 124)
            *cooked = KEY_RIGHT;
        else if (ev.code == 13)
            *cooked = KEY_ENTER;
    }

    *cooked |= (short)ev.state << KB_DOWN_SHF; // Add in the key-down state.

    // text input events are used to get printable characters (see sdl_events.c)
    // note that text input events don't work for ctrl'd or alt'd keys

    // If command-key was down, simulate a control key
    if (ev.modifiers & KB_MOD_CTRL)
        *cooked |= KB_FLAG_CTRL;

    // If shift-key was down
    if (ev.modifiers & KB_MOD_SHIFT)
        *cooked |= KB_FLAG_SHIFT;

    // If option-key was down, simulate an alt key.
    if (ev.modifiers & KB_MOD_ALT)
        *cooked |= KB_FLAG_ALT;

    return OK;
}
