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

#ifndef _LIGHT_H
#define _LIGHT_H

// prototypes
void check_for_near(void);
void scale_light_vec(void);
void g3_light_obj(g3s_phandle norm, g3s_phandle pos);
fix light_diff_raw(g3s_phandle src, g3s_phandle dest);
fix light_spec_raw(g3s_phandle src, g3s_phandle dest);
fix light_dands_raw(g3s_phandle src, g3s_phandle dest);

#endif // _LIGHT_H
