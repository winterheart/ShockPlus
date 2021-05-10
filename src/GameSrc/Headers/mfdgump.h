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

#ifndef MFDGUMP_H
#define MFDGUMP_H

// -----------
// PROTOTYPES
// -----------
void gump_clear(void);
uchar gump_pickup(byte row);
uchar gump_get_useful(bool shifted);
void mfd_gump_expose(MFD *mfd, ubyte control);
uchar mfd_gump_handler(MFD *m, uiEvent *uie);

extern uchar gump_num_objs;

#endif
