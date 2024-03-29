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
#ifndef __INPUT_H
#define __INPUT_H

#include "cursors.h"
#include "fix.h"
#include "frprotox.h"
#include "objects.h"
#include "region.h"

// -------
// DEFINES
// -------

#define NUM_MOTION_CURSORS 15

#define NUM_HOTKEYS 50 // an arbitrary constant

// input modes
#define INPUT_NORMAL_CURSOR 0
#define INPUT_OBJECT_CURSOR 1

#define MAX_JUMP_CONTROL (CONTROL_MAX_VAL / 2)

// ------
// PROTOS
// ------

/**
 * @deprecated does nothing
 */
void alloc_cursor_bitmaps(void);

/**
 * @deprecated does nothing
 */
void free_cursor_bitmaps();

void input_chk();
void shutdown_input();
void init_input();
void install_motion_mouse_handler(LGRegion *r, frc *fr);
void install_motion_keyboard_handler(LGRegion *r);
void look_at_object(ObjID id);
uchar posture_hotkey_func(ushort keycode, uint32_t context, intptr_t data);
void pop_cursor_object();
void push_cursor_object(short id);
void reload_motion_cursors(uchar cyber);
void reset_input_system();

char *get_object_lookname(ObjID id, char use_string[], int sz);

uchar check_object_dist(ObjID obj1, ObjID obj2, fix crit);

uchar toggle_opengl_func(ushort keycode, uint32_t context, intptr_t data);

uchar MacDetailFunc(ushort keycode, uint32_t context, intptr_t data);

void SetMotionCursorForMouseXY();

bool citadel_check_input();

// -------
// GLOBALS
// ------
extern ubyte fatigue_threshold;
extern int input_cursor_mode;
extern fix inpJoystickSens;
extern uchar joystick_count;
extern short object_on_cursor;

extern uchar mouse_jump_ui;
extern uchar fire_slam;
extern uchar left_down_jump;
extern uchar weapon_button_up;
extern LGCursor object_cursor;
extern LGPoint use_cursor_pos;
extern grs_bitmap motion_cursor_bitmaps[NUM_MOTION_CURSORS];

#endif
