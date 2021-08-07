/*

Copyright (C) 2018-2020 Shockolate Project
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

#pragma once

typedef struct MusicDevice MusicDevice;

typedef enum MusicType {
    Music_None,
#ifdef USE_LIBADLMIDI
    Music_AdlMidi,
#endif
    Music_Native,
#ifdef USE_FLUIDSYNTH
    Music_FluidSynth
#endif
} MusicType;

typedef enum MusicMode {
    Music_GeneralMidi,
    Music_SoundBlaster,
} MusicMode;

struct MusicDevice {
    int (*init)(MusicDevice *dev, const unsigned int outputIndex, unsigned samplerate);
    void (*destroy)(MusicDevice *dev);
    void (*setupMode)(MusicDevice *dev, MusicMode mode);
    void (*reset)(MusicDevice *dev);
    void (*generate)(MusicDevice *dev, short *samples, int numframes);
    void (*sendNoteOff)(MusicDevice *dev, int channel, int note, int vel);
    void (*sendNoteOn)(MusicDevice *dev, int channel, int note, int vel);
    void (*sendNoteAfterTouch)(MusicDevice *dev, int channel, int note, int touch);
    void (*sendControllerChange)(MusicDevice *dev, int channel, int ctl, int val);
    void (*sendProgramChange)(MusicDevice *dev, int channel, int pgm);
    void (*sendChannelAfterTouch)(MusicDevice *dev, int channel, int touch);
    void (*sendPitchBendML)(MusicDevice *dev, int channel, int msb, int lsb);
    unsigned int (*getOutputCount)(MusicDevice *dev);
    void (*getOutputName)(MusicDevice *dev, const unsigned int outputIndex, char *buffer,
                          const unsigned int bufferSize);
    unsigned short isOpen;    // 1 if device open, 0 if closed
    unsigned int outputIndex; // index of currently opened output
    MusicType deviceType;     // type of device
    const char *musicType;          // "sblaster" or "genmidi"
};

#define MUSICTYPE_SBLASTER "sblaster"
#define MUSICTYPE_GENMIDI "genmidi"

MusicDevice *CreateMusicDevice(MusicType type);
