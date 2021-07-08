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

#ifndef D_F_2_H
#define D_F_2_H

#include <stdint.h>

#include "fixpp.h"

void dirac_mechanicals(int32_t object, Q F[3], Q T[3]);

void mech_globalize(Q &X, Q &Y, Q &Z);

void mech_localize(Q &X, Q &Y, Q &Z);

#endif // D_F_2_H
