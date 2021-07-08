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

#ifndef _MATRIX_H
#define _MATRIX_H

void angles_2_matrix(g3s_angvec *angles, g3s_matrix *view_matrix, int rotation_order);
void process_view_matrix(void);
void scale_view_matrix(void);
void get_pyr_vector(g3s_vector *corners);

#endif // _MATRIX_H
