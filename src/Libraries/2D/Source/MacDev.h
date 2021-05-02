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
/*
 *
 * Prototypes and macros for general purpose Mac code.
 *
 * This file is part of the 2d library.
 *
 * Revision 1.1  1994/11/02  MLA
 * Initial revision
 */

#include "stdint.h"

#ifndef __MACDEV_H
#define __MACDEV_H
extern void (**mac_device_table[])();

extern void mac_set_mode(void);
extern void mac_set_pal (int start, int n, uint8_t *pal_data);
extern void mac_get_pal (int start, int n, uint8_t *pal_data);

#endif /* !__MACDEV_H */
