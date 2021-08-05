#pragma once
/*

Copyright (C) 2010-2016 OpenXcom Developers
Copyright (C) 2020-2021 ShockPlus Project

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

#include <string>
#include <vector>

#include "log.h"
#include "OptionInfo.h"

namespace ShockPlus::Options {

const int kMaxVolume = 100;

/// Audio log playback
enum AlogPlayback { ALOG_TEXT, ALOG_SPEECH, ALOG_BOTH };
/// Message Format for emails and text messages
enum MessageFormat { MESSAGE_NORMAL, MESSAGE_TERSE };
/// Supported MIDI backends
enum MIDIBackend { MIDI_ALMIDI, MIDI_NATIVE, MIDI_FLUIDSYNTH };
/// Quality of video
enum VideoDetail { VIDEO_DETAIL_MIN, VIDEO_DETAIL_LOW, VIDEO_DETAIL_HIGH, VIDEO_DETAIL_MAX };
/// Supported texture filters
enum VideoTextureFilter { VIDEO_TEXTURE_NONE, VIDEO_TEXTURE_BILINEAR };
/// Supported languages // FIXME: externalize
enum GeneralLanguage { LANGUAGE_ENGLISH, LANGUAGE_FRENCH, LANGUAGE_GERMAN };

#define OPT extern
#include "Options.inc.h"
#undef OPT

/// Creates the options info.
void create();
/// Restores default options.
void resetDefault();
/// Initializes the options settings.
bool init(int argc, char *argv[]);
/// Loads options from YAML.
bool load(const std::string &filename = "options.cfg");
/// Saves options to YAML.
bool save(const std::string &filename = "options.cfg");
/// Gets the game's data folder.
std::string getDataFolder();
/// Sets the game's data folder.
void setDataFolder(const std::string &folder);
/// Gets the game's data list.
const std::vector<std::string> &getDataList();
/// Gets the game's user folder.
std::string getUserFolder();
/// Gets the game's config folder.
std::string getConfigFolder();
/// Gets the game's options.
const std::vector<OptionInfo> &getOptionInfo();
/// Sets the game's data, user and config folders.
void setFolders();
/// Update game options from config file and command line.
void updateOptions();

}
