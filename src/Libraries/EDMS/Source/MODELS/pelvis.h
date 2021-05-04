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

#ifndef __PELVIS_H
#define __PELVIS_H

#include "fixpp.h"
#include "physhand.h"

//	This seems silly now, but later it will all make sense, sensei...
//	=================================================================
int32_t make_pelvis(Q init_state[6][3], Q params[10]);
void pelvis_set_control(int32_t pelvis, Q forward, Q turn, Q sidestep, Q lean, Q jump, int32_t crouch);
bool EDMS_pelvis_is_climbing();
void EDMS_lean_o_meter(physics_handle ph, fix &lean, fix &crouch);

#endif
