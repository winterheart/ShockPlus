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

#ifndef XMI_H
#define XMI_H

#include <SDL_mixer.h>

#include "MusicDevice.h"

#define NUM_THREADS 8

#define THREAD_INIT 0
#define THREAD_READY 1
#define THREAD_PLAYTRACK 2
#define THREAD_STOPTRACK 3
#define THREAD_EXIT 4

extern unsigned int NumTracks;

//-1: no thread is using this device channel;  0- : thread index that is using this device channel
extern char ChannelThread[16]; // 16 device channels

extern int NumUsedChannels; // number of in-use device channels

void AudioStreamCallback(void *userdata, unsigned char *stream, int len);
void MusicCallback(void *userdata, Uint8 *stream, int len);

void FreeXMI(void);
int ReadXMI(const char *filename);
void StartTrack(int i, unsigned int track);
void StopTrack(int i);
void StopTheMusic(void);
int IsPlaying(int i);
void InitReadXMI(void);
void InitDecXMI(void);
void ReloadDecXMI(void);
void ShutdownReadXMI(void);
unsigned int GetOutputCountXMI(void);
void GetOutputNameXMI(const unsigned int outputIndex, char *buffer, const unsigned int bufferSize);
void UpdateVolumeXMI(void);

struct midi_event_struct {
    int time;
    unsigned char status;
    unsigned char data[2];
    unsigned int len;
    unsigned char *buffer;
    struct midi_event_struct *next;
};

typedef struct midi_event_struct MIDI_EVENT;

extern MIDI_EVENT **TrackEvents;
extern short *TrackTiming;
extern unsigned short *TrackUsedChannels;

extern MIDI_EVENT *ThreadEventList[NUM_THREADS];
extern int ThreadTiming[NUM_THREADS];
extern char ThreadChannelRemap[16 * NUM_THREADS];
extern SDL_atomic_t DeviceChannelVolume[16]; // only msb: 0-127
extern SDL_atomic_t ThreadPlaying[NUM_THREADS];
extern SDL_atomic_t ThreadCommand[NUM_THREADS];

extern MusicDevice *MusicDev;

struct thread_data {
    int i; // thread index
};

#endif
