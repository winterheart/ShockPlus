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
 * $Source: r:/prj/lib/src/input/RCS/kbcook.h $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1994/08/15 16:36:40 $
 *
 * Constants for cooked keyboard event codes.
 *
 * This file is part of the input library.
 */

#ifndef __KBCOOK_H
#define __KBCOOK_H

#include <ctype.h>
#include "lg_error.h"
#include "kb.h"

#define CNV_CTRL (1 << (1 + 8))    // KB_FLAG_CTRL can be set
#define CNV_ALT (1 << (2 + 8))     // KB_FLAG_SHIFT can be set
#define CNV_SPECIAL (1 << (3 + 8)) // KB_FLAG_SPECIAL is set
#define CNV_SHIFT (1 << (4 + 8))   // KB_FLAG_SHIFT can be set
#define CNV_2ND (1 << (5 + 8))     // KB_FLAG_2ND is set
#define CNV_NUM (1 << (6 + 8))     // affected by numlock
#define CNV_CAPS (1 << (7 + 8))    // affected by capslock

#define CNV_CAPS_SHF (7 + 8)

#define KB_FLAG_DOWN (1 << (0 + 8))
#define KB_FLAG_CTRL CNV_CTRL
#define KB_FLAG_ALT CNV_ALT
#define KB_FLAG_SPECIAL CNV_SPECIAL
#define KB_FLAG_SHIFT CNV_SHIFT
#define KB_FLAG_2ND CNV_2ND

#define KB_DOWN_SHF 8
#define KB_CTRL_SHF 9
#define KB_ALT_SHF 10
#define KB_SPECIAL_SHF 11
#define KB_SHIFT_SHF 12
#define KB_2ND_SHF 13

// "cooks" kb event "code."  If cooking generates a cooked code, sets
// *results to true and puts the result in *cooked.  Otherwise, *results = false.
errtype kb_cook(kbs_event code, ushort *cooked, uchar *results);

#define kb2ascii(x) (((x)&KB_FLAG_SPECIAL) ? 0 : (x)&0xFF)

#endif /* __KBCOOK_H */
