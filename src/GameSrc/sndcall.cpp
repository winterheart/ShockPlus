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
 * $Source: r:/prj/cit/src/RCS/sndcall.c $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/11/28 08:31:43 $
 */

#include "digifx.h"
#include "musicai.h"

#define MAX_UNLOCK 32
int rulock_list[MAX_UNLOCK];
int rulock_ptr = 0;

/* KLC - not used in Mac version.
void cdecl simple_xmi_stop(snd_midi_parms *seq)
{
//   if (seq->snd_ref==0xc1c1)
           Free(seq->data);
   mono_ch(60,'A'+tmp);
   tlc(tmp=(tmp+1)&0xf);
   mono_ch(61,'a'+simple_xmi_sound_on);
   simple_xmi_sound_on--;
}
*/

void digifx_EOS_callback(snd_digi_parms *sdp) {
    /*if (sdp->snd_ref>0x10)
            if (rulock_ptr<MAX_UNLOCK-1)
                    rulock_list[rulock_ptr++]=sdp->snd_ref;*/
}

void sound_frame_update(void) {
    int i;
    snd_digi_parms *sdp;

    for (i = 0; i < SND_MAX_SAMPLES; i++) {
        snd_digi_parms *sdp = snd_sample_parms(i);
        if (set_sample_pan_gain(sdp))
            snd_end_sample(i);
    }
}
