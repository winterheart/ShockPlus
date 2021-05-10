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
#ifndef __OBJUSE_H
#define __OBJUSE_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/objuse.h $
 * $Revision: 1.13 $
 * $Author: xemu $
 * $Date: 1994/08/22 03:04:22 $
 *
 *
 */

// Includes
#include "lg_error.h"
#include "objects.h"

// Defines

#define NUM_ACCESS_CODES 32

#define PICKUP_USE_MODE 0
#define USE_USE_MODE 1
#define NULL_USE_MODE (INVENT_USEMODE >> INVENT_USEMODE_SHF)

extern ubyte use_distance_mod;
extern ubyte pickup_distance_mod;

#define BASE_PICKUP_DIST (FIX_UNIT * 20 / 8)
#define BASE_USE_DIST BASE_PICKUP_DIST

#define MAX_PICKUP_DIST (BASE_PICKUP_DIST + fix_make(pickup_distance_mod, 0))
#define MAX_USE_DIST (BASE_USE_DIST + fix_make(use_distance_mod, 0))

#define USE_MODE(x) ((ObjProps[OPNUM((x))].flags & INVENT_USEMODE) >> INVENT_USEMODE_SHF)

// Typedefs
typedef struct {
    ushort timestamp;
    ushort type;
    ObjID door_id;
    ushort secret_code;
} DoorSchedEvent;

// Prototypes

// Access panels stick their door to be opened in P1
errtype accesspanel_trigger(ObjID id);

// Hey.  You've been USED, man.  Prove your manhood. Do something about it!
// Of course for now, do nothing if not of CLASS_FIXTURE.
// Returns whether or not the message line was used.
uchar object_use(ObjID id, uchar in_inv, ObjID cursor_obj);

// Lock/unlock a door
errtype obj_door_lock(ObjID door_id, uchar new_lock);

errtype obj_screen_animate(ObjID id);

errtype obj_tractor_beam_func(ObjID id, uchar on);

char container_extract(ObjID *pidlist, int d1, int d2);

void container_stuff(ObjID *pidlist, int numobjs, int *d1, int *d2);

uchar is_container(ObjID id, int **d1, int **d2);

errtype keypad_trigger(ObjID id, uchar digits[]);

// Special case for elevator
uchar elevator_use(short dest_level, ubyte which_panel);

errtype obj_cspace_collide(ObjID id, ObjID collider);

uchar obj_too_smart(ObjID id);

void multi_anim_callback(ObjID id, intptr_t user_data);

void unmulti_anim_callback(ObjID id, intptr_t user_data);

errtype gear_power_outage();

void regenetron_door_hack();

// Globals

//set if shift key was held when using object
extern bool ObjectUseShifted;
extern uchar shameful_obselete_flag;

#endif // __OBJUSE_H
