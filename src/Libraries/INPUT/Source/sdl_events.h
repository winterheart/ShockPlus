/*

Copyright (C) 2020 Shockolate Project

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

#ifndef SDL_EVENT_H
#define SDL_EVENT_H

// Ingest SDL events into internal Event system
void pump_events(void);

void get_mouselook_vel(int *vx, int *vy);
void set_mouse_chaos(short dx, short dy);
void middleize_mouse(void);

extern bool fullscreenActive;

#endif
