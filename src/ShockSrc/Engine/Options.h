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

#include <filesystem>
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

/// Keycode Aliases
enum KeyCodes {
    KEY_CANCEL_AUDIOLOG,
    KEY_STAND,
    KEY_CROUCH,
    KEY_PRONE,
    KEY_TOGGLE_FREELOOK,
    KEY_FULL_VIEW,
    KEY_NORMAL_VIEW,
    KEY_MAP_VIEW,
    KEY_CLEAR_FULLSCREEN,
    KEY_SAVE_GAME,
    KEY_LOAD_GAME,
    KEY_PAUSE,
    KEY_RELOAD_WEAPON_1,
    KEY_RELOAD_WEAPON_0,
    KEY_SELECT_GRENADE,
    KEY_SELECT_DRUG,
    KEY_TOGGLE_OLH,
    KEY_TOGGLE_MUSIC,
    KEY_QUIT,
    KEY_NEXT_WEAPON,
    KEY_PREV_WEAPON,
    KEY_CYCLE_DETAIL,
    KEY_TOGGLE_OPENGL,
    KEY_ARM_GRENADE,
    KEY_USE_DRUG,
    KEY_HUD_COLOR,
    KEY_SHOW_HELP,
    KEY_BIOSCAN,
    KEY_FULLSCREEN,
    KEY_360_VIEW,
    KEY_LATERN,
    KEY_SHIELD,
    KEY_INFRARED,
    KEY_NAV_UNIT,
    KEY_DATA_READER,
    KEY_BOOSTER,
    KEY_JUMPJETS,
    KEY_MFD_L1,
    KEY_MFD_L2,
    KEY_MFD_L3,
    KEY_MFD_L4,
    KEY_MFD_L5,
    KEY_MFD_R1,
    KEY_MFD_R2,
    KEY_MFD_R3,
    KEY_MFD_R4,
    KEY_MFD_R5,
    KEY_NUMPAD_0,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    KEY_TOGGLE_OPTIONS,
    KEY_CHEAT_GIVEALL,
    KEY_CHEAT_PHYSICS,
    KEY_CHEAT_LEVEL_UP,
    KEY_CHEAT_LEVEL_DOWN,
    KEY_COUNT
};

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
std::filesystem::path getDataFolder();
/// Sets the game's data folder.
void setDataFolder(const std::filesystem::path &folder);
/// Gets the game's data list.
const std::vector<std::filesystem::path> &getDataList();
/// Gets the game's user folder.
std::filesystem::path getUserFolder();
/// Gets the game's config folder.
std::filesystem::path getConfigFolder();
/// Gets the game's saves folder
std::filesystem::path getSavesFolder();
/// Gets the game's options.
const std::vector<OptionInfo> &getOptionInfo();
/// Sets the game's data, user and config folders.
void setFolders();
/// Update game options from config file and command line.
void updateOptions();

} // namespace ShockPlus::Options
