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
#ifndef __HOTKEY_H
#define __HOTKEY_H

/*
 * $Source: n:/project/lib/src/ui/RCS/hotkey.h $
 * $Revision: 1.6 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:20 $
 *
 * $Log: hotkey.h $
 * Revision 1.6  1993/10/11  20:27:20  dc
 * Angle is fun, fun fun fun
 *
 * Revision 1.5  1993/06/14  21:50:15  xemu
 * export structures
 *
 * Revision 1.4  1993/06/14  21:12:08  xemu
 * failed list
 *
 * Revision 1.3  1993/05/17  15:52:21  xemu
 * help text
 *
 * Revision 1.2  1993/04/28  14:40:17  mahk
 * Preparing for second exodus
 *
 * Revision 1.1  1993/03/26  21:50:33  mahk
 * Initial revision
 *
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

// Includes
#include "lg.h" // every file should have this
#include "lg_error.h"
#include "hash.h"
#include "kbcook.h"
#include "array.h"

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

#define HKSORT_NONE 0
#define HKSORT_KEYCODE 1
#define HKSORT_ASCII 2

typedef uchar (*hotkey_callback)(ushort keycode, uint32_t context, intptr_t state);

typedef struct _hotkey_entry {
    ushort key;
    Array keychain;
    int first;
} hotkey_entry;

typedef struct _hotkey_link {
    uint32_t context;
    hotkey_callback func;
    intptr_t state;
    int next;
} hotkey_link;

extern Hashtable hotkey_table;

// Prototypes

// Initialize hotkey table, giving an initial context and table size.
errtype hotkey_init(int tblsize);

// installs a hotkey handler for a specific cooked keycode in the set of contexts described by context_mask.
// This handler will take precidence over previously-installed handlers.
errtype hotkey_add(ushort keycode, uint32_t context_mask, hotkey_callback func, intptr_t state);

// delete all hotkey handlers with the specified keycode and callback function
// from the contexts specified by the context_mask.
errtype hotkey_remove(short keycode, ulong context_mask, hotkey_callback func);

// dispatches the keycode to the highest-priority key handler for that
// keycode that has any set bits in common with HotkeyContext.
errtype hotkey_dispatch(short keycode);

// shut down the hotkey system.
errtype hotkey_shutdown(void);

// Globals

extern uint32_t HotkeyContext;

#ifdef __cplusplus
}
#endif

#endif // __HOTKEY_H
