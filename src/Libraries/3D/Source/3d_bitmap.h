/*

Copyright (C) 2021 ShockPlus Project

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

#ifndef _3D_BITMAP_H
#define _3D_BITMAP_H

#include <stdbool.h>

// prototypes

bool SubLongWithOverflow(int32_t *result, int32_t src, int32_t dest);
bool AddLongWithOverflow(int32_t *result, int32_t src, int32_t dest);

#endif // _3D_BITMAP_H
