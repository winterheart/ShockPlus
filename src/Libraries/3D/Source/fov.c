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
//
// $Source: r:/prj/lib/src/3d/RCS/fov.asm $
// $Revision: 1.4 $
// $Author: jaemz $
// $Date: 1994/10/26 21:30:21 $
//
// Routines to get FOV and zoom
//
// $Log: fov.asm $
// Revision 1.4  1994/10/26  21:30:21  jaemz
// Added get_zoom refresh aspect rat from 2d
//
// Revision 1.3  1994/06/02  15:07:37  junochoe
// changed matrix_scale to _matrix_scale
//
//
// Revision 1.2  1993/08/10  22:54:07  dc
// add _3d.inc to includes
//
// Revision 1.1  1993/05/04  17:39:45  matt
// Initial revision
//
//

#include "3d.h"
#include "globalv.h"
#include "lg.h"

// returns zoom for a desired FOV.
// takes bx=FOV angle, al=axis ('X' or 'Y'), ecx=window width, edx=window height
// returns in eax. trashes all but ebp

fix g3_get_zoom(char axis, fixang angle, int user_window_width, int user_window_height) {
    fix sin_val, cos_val;
    fix unscalezoom, temp1;
    int32_t templong;

    fix_sincos(angle, &sin_val, &cos_val);
    temp1 = fix_div(FIX_UNIT - cos_val, cos_val + FIX_UNIT);

    unscalezoom = fix_sqrt(temp1);
    // now, temp1 would be zoom if not for window and pixel matrix scaling.
    // correct for these

    // get pixel ratio
    pixel_ratio = grd_cap->aspect;

    // get matrix scale value for given window size
    templong = fix_mul_div(user_window_height, pixel_ratio, user_window_width);
    // window and pixrat scaling affects y. see if y FOV requested
    if (templong <= FIX_UNIT) {
        if (axis != 'X')
            return (unscalezoom);
        return (fix_mul(unscalezoom, templong));
    } else {
        if (axis != 'Y')
            return (unscalezoom);
        return (fix_div(unscalezoom, templong));
    }
}
