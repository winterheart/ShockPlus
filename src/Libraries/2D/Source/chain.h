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
 * $Source: n:/project/lib/src/2d/RCS/chain.h $
 * $Revision: 1.8 $
 * $Author: baf $
 * $Date: 1993/12/08 19:14:57 $
 *
 * Prototypes and macros for function chaining.
 *
 * This file is part of the 2D library.
 *
 * $Log: chain.h $
 * Revision 1.8  1993/12/08  19:14:57  baf
 * Fixed bug in gr_toggle_generic
 * 
 * Revision 1.7  1993/12/07  11:46:34  baf
 * Renamed gr_chain_add
 * 
 * Revision 1.6  1993/12/02  13:44:45  baf
 * Added the ability to unchain and rechain.
 * 
 * Revision 1.5  1993/11/30  20:47:24  baf
 * Chainged generic mode stuff so it can
 * be done at any time.
 * 
 * Revision 1.4  1993/11/30  19:31:22  baf
 * Added more macros for manipulating
 * chaining and generic mode
 * 
 * Revision 1.3  1993/11/16  23:06:58  baf
 * Added the ability to chain void functions
 * after the primitive.
 * 
 * Revision 1.2  1993/11/15  03:28:49  baf
 * Added gr_chain_add_void, as well as
 * support for forcing generic mode and
 * turning chaining off.
 * 
 * Revision 1.1  1993/11/12  09:30:18  baf
 * Initial revision
 * 
 */

#ifndef __CHAIN
#define __CHAIN

#include "grs.h"
#include "icanvas.h"

extern short grd_pixel_index;
extern short grd_canvas_index;

extern uchar chn_flags;
#define CHN_ON 1
#define CHN_GEN 2

#define gr_generic (chn_flags & CHN_GEN)

#endif
