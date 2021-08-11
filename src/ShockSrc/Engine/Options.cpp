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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <SDL_filesystem.h>

#include "Options.h"
#include "lg.h"
#include "shock_version.h"

namespace ShockPlus::Options {

#define OPT
#include "Options.inc.h"
#undef OPT

std::filesystem::path dataFolder_;
std::vector<std::filesystem::path> dataList_;
std::filesystem::path userFolder_;
std::filesystem::path configFolder_;
std::map<std::string, std::string> commandLine_;
std::vector<OptionInfo> info_;
std::vector<OptionInfo> keys_;

/**
 * Write node from emitter
 * @param node Node
 * @param emitter Emmiter
 */
void writeNode(const YAML::Node &node, YAML::Emitter &emitter) {
    switch (node.Type()) {
    case YAML::NodeType::Sequence: {
        emitter << YAML::BeginSeq;
        for (const auto &i : node) {
            writeNode(i, emitter);
        }
        emitter << YAML::EndSeq;
        break;
    }
    case YAML::NodeType::Map: {
        emitter << YAML::BeginMap;

        // First collect all the keys
        std::vector<std::string> keys(node.size());
        int key_it = 0;
        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
            keys[key_it++] = it->first.as<std::string>();
        }

        // Then sort them
        std::sort(keys.begin(), keys.end());

        // Then emit all the entries in sorted order.
        for (auto &key : keys) {
            emitter << YAML::Key;
            emitter << key;
            emitter << YAML::Value;
            writeNode(node[key], emitter);
        }
        emitter << YAML::EndMap;
        break;
    }
    default:
        emitter << node;
        break;
    }
}

/**
 * Returns the game's Config folder where settings are stored in. Normally the
 * same as the User folder.
 * @return Full path to Config folder.
 */
std::filesystem::path getConfigFolder() { return configFolder_; }

/**
 * Returns the game's current Data folder where resources are loaded from.
 * @return Full path to Data folder.
 */
std::filesystem::path getDataFolder() { return dataFolder_; }

/**
 * Returns the game's User folder where user's data are stored in.
 * @return Full path to User folder.
 */
std::filesystem::path getUserFolder() { return userFolder_; }

/**
 * Returns the game's Saves folder where saves are stored in.
 * @return
 */
std::filesystem::path getSavesFolder() { return (userFolder_ / "saves"); };

/**
 * Returns the game's list of all available option information.
 * @return List of OptionInfo's.
 */
const std::vector<OptionInfo> &getOptionInfo() { return info_; }

/**
 * Returns the game's list of possible Data folders.
 * @return List of Data paths.
 */
const std::vector<std::filesystem::path> &getDataList() { return dataList_; }

/**
 * Changes the game's current Data folder where resources are loaded from.
 * @param folder Full path to Data folder.
 */
void setDataFolder(const std::filesystem::path &folder) { dataFolder_ = folder; }

/**
 * Sets up the options by creating their OptionInfo metadata.
 */
void create() {
    // Initialize
    for (uint32_t i = 0; i < ShockPlus::Options::KeyCodes::KEY_COUNT; i++) {
        keyMap.insert_or_assign(i, KeyDef{{.scancode = SDL_SCANCODE_UNKNOWN, .mod = KMOD_NONE}});
    }

    // First goes non-UI options
    info_.emplace_back("logLevel", (int *)&logLevel, LOG_INFO);

    // Control options
    info_.emplace_back("captureMouse", &captureMouse, true, "STR_OPTION_CAPTURE_MOUSE", "STR_OPTIONS_CONTROLS");
    info_.emplace_back("invertMouseY", &invertMouseY, false, "STR_OPTION_INVERT_MOUSE_Y", "STR_OPTIONS_CONTROLS");

    // General options
    info_.emplace_back("language", (int *)&language, LANGUAGE_ENGLISH, "STR_OPTION_LANGUAGE", "STR_OPTIONS_GENERAL");
    info_.emplace_back("messageFormat", (int *)&messageFormat, MESSAGE_NORMAL, "STR_OPTION_MESSAGE_FORMAT",
                       "STR_OPTIONS_GENERAL");
    info_.emplace_back("showIntro", &showIntro, false, "STR_OPTION_SHOW_INTRO", "STR_OPTIONS_GENERAL");
    info_.emplace_back("showOnScreenHelp", &showOnScreenHelp, true, "STR_OPTION_SHOW_ONSCREEN_HELP",
                       "STR_OPTIONS_GENERAL");
    info_.emplace_back("showTooltipMessages", &showTooltipMessages, true, "STR_OPTION_SHOW_TOOLTIP_MESSAGES",
                       "STR_OPTIONS_GENERAL");
    info_.emplace_back("showSplash", &showSplash, false, "STR_OPTION_SHOW_BOOT_SPLASHES", "STR_OPTIONS_GENERAL");

    // Sound options
    info_.emplace_back("alogPlayback", (int *)&alogPlayback, ALOG_BOTH, "STR_OPTION_ALOG_PLAYBACK",
                       "STR_OPTIONS_SOUND");
    info_.emplace_back("enableSFX", &enableSFX, true, "STR_OPTION_ENABLE_SFX", "STR_OPTIONS_SOUND");
    info_.emplace_back("enableSound", &enableSound, true, "STR_OPTION_ENABLE_SOUND", "STR_OPTIONS_SOUND");
    info_.emplace_back("midiBackend", (int *)&midiBackend, MIDI_NATIVE, "STR_OPTION_MIDI_BACKEND", "STR_OPTIONS_SOUND");
    info_.emplace_back("midiOutput", &midiOutput, 0, "STR_MIDI_OUTPUT", "STR_OPTIONS_SOUND");
    info_.emplace_back("musicVolume", &musicVolume, 3 * kMaxVolume / 4, "STR_OPTION_MUSIC_VOLUME", "STR_OPTIONS_SOUND");
    info_.emplace_back("sfxVolume", &sfxVolume, 3 * kMaxVolume / 4, "STR_OPTION_SFX_VOLUME", "STR_OPTIONS_SOUND");
    info_.emplace_back("voiceVolume", &voiceVolume, 3 * kMaxVolume / 4, "STR_OPTION_VOICE_VOLUME", "STR_OPTIONS_SOUND");

    // Video options
    info_.emplace_back("videoMode", &videoMode, 3, "STR_OPTION_VIDEO_MODE", "STR_OPTIONS_VIDEO");
    info_.emplace_back("enableOpenGL", &enableOpenGL, false, "STR_OPTION_ENABLE_OPENGL", "STR_OPTIONS_VIDEO");
    info_.emplace_back("enableSkipLines", &enableSkipLines, false, "STR_OPTION_ENABLE_SKIPLINES", "STR_OPTIONS_VIDEO");
    info_.emplace_back("gammaCorrection", &gammaCorrection, 3 * kMaxVolume / 4, "STR_OPTION_VOICE_VOLUME",
                       "STR_OPTIONS_VIDEO");
    info_.emplace_back("halfResolution", &halfResolution, false, "STR_OPTION_HALF_RESOLUTION", "STR_OPTIONS_VIDEO");
    info_.emplace_back("videoDetail", (int *)&videoDetail, VIDEO_DETAIL_MAX, "STR_OPTION_VIDEO_DETAIL",
                       "STR_OPTIONS_VIDEO");
    info_.emplace_back("videoTextureFilter", (int *)&videoTextureFilter, VIDEO_TEXTURE_NONE,
                       "STR_OPTION_TEXTURE_FILTER", "STR_OPTIONS_VIDEO");

    // keybindings
    keys_.emplace_back("KEY_CANCEL_AUDIOLOG", &keyMap[KeyCodes::KEY_CANCEL_AUDIOLOG],
                       KeyDef{{.scancode = SDL_SCANCODE_PERIOD, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_AUDIOLOG_CANCEL",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_STAND", &keyMap[KeyCodes::KEY_STAND],
                       KeyDef{{.scancode = SDL_SCANCODE_T, .mod = KMOD_NONE}}, "STR_OPTION_KEY_STAND",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CROUCH", &keyMap[KeyCodes::KEY_CROUCH],
                       KeyDef{{.scancode = SDL_SCANCODE_G, .mod = KMOD_NONE}}, "STR_OPTION_KEY_CROUCH",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_PRONE", &keyMap[KeyCodes::KEY_PRONE],
                       KeyDef{{.scancode = SDL_SCANCODE_B, .mod = KMOD_NONE}}, "STR_OPTION_KEY_PRONE",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_TOGGLE_FREELOOK", &keyMap[KeyCodes::KEY_TOGGLE_FREELOOK],
                       KeyDef{{.scancode = SDL_SCANCODE_F, .mod = KMOD_NONE}}, "STR_OPTION_KEY_TOGGLE_FREELOOK",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_FULL_VIEW", &keyMap[KeyCodes::KEY_FULL_VIEW],
                       KeyDef{{.scancode = SDL_SCANCODE_F, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_FULL_VIEW",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NORMAL_VIEW", &keyMap[KeyCodes::KEY_NORMAL_VIEW],
                       KeyDef{{.scancode = SDL_SCANCODE_D, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_NORMAL_VIEW",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MAP_VIEW", &keyMap[KeyCodes::KEY_MAP_VIEW],
                       KeyDef{{.scancode = SDL_SCANCODE_A, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_MAP_VIEW",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CLEAR_FULLSCREEN", &keyMap[KeyCodes::KEY_CLEAR_FULLSCREEN],
                       KeyDef{{.scancode = SDL_SCANCODE_BACKSPACE, .mod = KMOD_NONE}},
                       "STR_OPTION_KEY_CLEAR_FULLSCREEN", "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_SAVE_GAME", &keyMap[KeyCodes::KEY_SAVE_GAME],
                       KeyDef{{.scancode = SDL_SCANCODE_S, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_SAVE_GAME",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_LOAD_GAME", &keyMap[KeyCodes::KEY_LOAD_GAME],
                       KeyDef{{.scancode = SDL_SCANCODE_L, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_LOAD_GAME",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_PAUSE", &keyMap[KeyCodes::KEY_PAUSE],
                       KeyDef{{.scancode = SDL_SCANCODE_P, .mod = KMOD_NONE}}, "STR_OPTION_KEY_PAUSE",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_RELOAD_WEAPON_1", &keyMap[KeyCodes::KEY_RELOAD_WEAPON_1],
                       KeyDef{{.scancode = SDL_SCANCODE_BACKSPACE, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_RELOAD_WEAPON_1",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_RELOAD_WEAPON_0", &keyMap[KeyCodes::KEY_RELOAD_WEAPON_0],
                       KeyDef{{.scancode = SDL_SCANCODE_BACKSPACE, .mod = KMOD_ALT}}, "STR_OPTION_KEY_RELOAD_WEAPON_0",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_SELECT_GRENADE", &keyMap[KeyCodes::KEY_SELECT_GRENADE],
                       KeyDef{{.scancode = SDL_SCANCODE_APOSTROPHE, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_SELECT_GRENADE",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_SELECT_DRUG", &keyMap[KeyCodes::KEY_SELECT_DRUG],
                       KeyDef{{.scancode = SDL_SCANCODE_SEMICOLON, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_SELECT_DRUG",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_TOGGLE_OLH", &keyMap[KeyCodes::KEY_TOGGLE_OLH],
                       KeyDef{{.scancode = SDL_SCANCODE_H, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_TOGGLE_OLH",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_TOGGLE_MUSIC", &keyMap[KeyCodes::KEY_TOGGLE_MUSIC],
                       KeyDef{{.scancode = SDL_SCANCODE_M, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_TOGGLE_MUSIC",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_QUIT", &keyMap[KeyCodes::KEY_QUIT], KeyDef{{.scancode = SDL_SCANCODE_Q, .mod = KMOD_CTRL}},
                       "STR_OPTION_KEY_QUIT", "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NEXT_WEAPON", &keyMap[KeyCodes::KEY_NEXT_WEAPON],
                       KeyDef{{.scancode = SDL_SCANCODE_TAB, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NEXT_WEAPON",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_PREV_WEAPON", &keyMap[KeyCodes::KEY_PREV_WEAPON],
                       KeyDef{{.scancode = SDL_SCANCODE_TAB, .mod = KMOD_SHIFT}}, "STR_OPTION_KEY_PREV_WEAPON",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CYCLE_DETAIL", &keyMap[KeyCodes::KEY_CYCLE_DETAIL],
                       KeyDef{{.scancode = SDL_SCANCODE_1, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_CYCLE_DETAIL",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_TOGGLE_OPENGL", &keyMap[KeyCodes::KEY_TOGGLE_OPENGL],
                       KeyDef{{.scancode = SDL_SCANCODE_G, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_TOGGLE_OPENGL",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_ARM_GRENADE", &keyMap[KeyCodes::KEY_ARM_GRENADE],
                       KeyDef{{.scancode = SDL_SCANCODE_APOSTROPHE, .mod = KMOD_ALT}}, "STR_OPTION_KEY_ARM_GRENADE",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_USE_DRUG", &keyMap[KeyCodes::KEY_USE_DRUG],
                       KeyDef{{.scancode = SDL_SCANCODE_SEMICOLON, .mod = KMOD_ALT}}, "STR_OPTION_KEY_USE_DRUG",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_HUD_COLOR", &keyMap[KeyCodes::KEY_HUD_COLOR],
                       KeyDef{{.scancode = SDL_SCANCODE_H, .mod = KMOD_ALT}}, "STR_OPTION_KEY_HUD_COLOR",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_SHOW_HELP", &keyMap[KeyCodes::KEY_SHOW_HELP],
                       KeyDef{{.scancode = SDL_SCANCODE_O, .mod = KMOD_ALT}}, "STR_OPTION_KEY_SHOW_HELP",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_BIOSCAN", &keyMap[KeyCodes::KEY_BIOSCAN],
                       KeyDef{{.scancode = SDL_SCANCODE_1, .mod = KMOD_NONE}}, "STR_OPTION_KEY_BIOSCAN",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_FULLSCREEN", &keyMap[KeyCodes::KEY_FULLSCREEN],
                       KeyDef{{.scancode = SDL_SCANCODE_2, .mod = KMOD_NONE}}, "STR_OPTION_KEY_FULLSCREEN",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_360_VIEW", &keyMap[KeyCodes::KEY_360_VIEW],
                       KeyDef{{.scancode = SDL_SCANCODE_3, .mod = KMOD_NONE}}, "STR_OPTION_KEY_360_VIEW",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_LATERN", &keyMap[KeyCodes::KEY_LATERN],
                       KeyDef{{.scancode = SDL_SCANCODE_4, .mod = KMOD_NONE}}, "STR_OPTION_KEY_LATERN",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_SHIELD", &keyMap[KeyCodes::KEY_SHIELD],
                       KeyDef{{.scancode = SDL_SCANCODE_5, .mod = KMOD_NONE}}, "STR_OPTION_KEY_SHIELD",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_INFRARED", &keyMap[KeyCodes::KEY_INFRARED],
                       KeyDef{{.scancode = SDL_SCANCODE_6, .mod = KMOD_NONE}}, "STR_OPTION_KEY_INFRARED",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NAV_UNIT", &keyMap[KeyCodes::KEY_NAV_UNIT],
                       KeyDef{{.scancode = SDL_SCANCODE_7, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NAV_UNIT",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_DATA_READER", &keyMap[KeyCodes::KEY_DATA_READER],
                       KeyDef{{.scancode = SDL_SCANCODE_8, .mod = KMOD_NONE}}, "STR_OPTION_KEY_DATA_READER",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_BOOSTER", &keyMap[KeyCodes::KEY_BOOSTER],
                       KeyDef{{.scancode = SDL_SCANCODE_9, .mod = KMOD_NONE}}, "STR_OPTION_KEY_BOOSTER",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_JUMPJETS", &keyMap[KeyCodes::KEY_JUMPJETS],
                       KeyDef{{.scancode = SDL_SCANCODE_0, .mod = KMOD_NONE}}, "STR_OPTION_KEY_JUMPJETS",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_L1", &keyMap[KeyCodes::KEY_MFD_L1],
                       KeyDef{{.scancode = SDL_SCANCODE_F1, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_L1",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_L2", &keyMap[KeyCodes::KEY_MFD_L2],
                       KeyDef{{.scancode = SDL_SCANCODE_F2, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_L2",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_L3", &keyMap[KeyCodes::KEY_MFD_L3],
                       KeyDef{{.scancode = SDL_SCANCODE_F3, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_L3",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_L4", &keyMap[KeyCodes::KEY_MFD_L4],
                       KeyDef{{.scancode = SDL_SCANCODE_F4, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_L4",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_L5", &keyMap[KeyCodes::KEY_MFD_L5],
                       KeyDef{{.scancode = SDL_SCANCODE_F5, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_L5",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_R1", &keyMap[KeyCodes::KEY_MFD_R1],
                       KeyDef{{.scancode = SDL_SCANCODE_F6, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_R1",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_R2", &keyMap[KeyCodes::KEY_MFD_R2],
                       KeyDef{{.scancode = SDL_SCANCODE_F7, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_R2",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_R3", &keyMap[KeyCodes::KEY_MFD_R3],
                       KeyDef{{.scancode = SDL_SCANCODE_F8, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_R3",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_R4", &keyMap[KeyCodes::KEY_MFD_R4],
                       KeyDef{{.scancode = SDL_SCANCODE_F9, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_R4",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_MFD_R5", &keyMap[KeyCodes::KEY_MFD_R5],
                       KeyDef{{.scancode = SDL_SCANCODE_F10, .mod = KMOD_NONE}}, "STR_OPTION_KEY_MFD_R5",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_0", &keyMap[KeyCodes::KEY_NUMPAD_0],
                       KeyDef{{.scancode = SDL_SCANCODE_O, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_0",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_1", &keyMap[KeyCodes::KEY_NUMPAD_1],
                       KeyDef{{.scancode = SDL_SCANCODE_1, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_1",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_2", &keyMap[KeyCodes::KEY_NUMPAD_2],
                       KeyDef{{.scancode = SDL_SCANCODE_2, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_2",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_3", &keyMap[KeyCodes::KEY_NUMPAD_3],
                       KeyDef{{.scancode = SDL_SCANCODE_3, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_3",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_4", &keyMap[KeyCodes::KEY_NUMPAD_4],
                       KeyDef{{.scancode = SDL_SCANCODE_4, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_4",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_5", &keyMap[KeyCodes::KEY_NUMPAD_5],
                       KeyDef{{.scancode = SDL_SCANCODE_5, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_5",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_6", &keyMap[KeyCodes::KEY_NUMPAD_6],
                       KeyDef{{.scancode = SDL_SCANCODE_6, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_6",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_7", &keyMap[KeyCodes::KEY_NUMPAD_7],
                       KeyDef{{.scancode = SDL_SCANCODE_7, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_7",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_8", &keyMap[KeyCodes::KEY_NUMPAD_8],
                       KeyDef{{.scancode = SDL_SCANCODE_8, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_8",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_NUMPAD_9", &keyMap[KeyCodes::KEY_NUMPAD_9],
                       KeyDef{{.scancode = SDL_SCANCODE_9, .mod = KMOD_NONE}}, "STR_OPTION_KEY_NUMPAD_9",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_TOGGLE_OPTIONS", &keyMap[KeyCodes::KEY_TOGGLE_OPTIONS],
                       KeyDef{{.scancode = SDL_SCANCODE_ESCAPE, .mod = KMOD_NONE}}, "STR_OPTION_KEY_TOGGLE_OPTIONS",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CHEAT_GIVEALL", &keyMap[KeyCodes::KEY_CHEAT_GIVEALL],
                       KeyDef{{.scancode = SDL_SCANCODE_2, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_CHEAT_GIVEALL",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CHEAT_PHYSICS", &keyMap[KeyCodes::KEY_CHEAT_PHYSICS],
                       KeyDef{{.scancode = SDL_SCANCODE_3, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_CHEAT_PHYSICS",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CHEAT_LEVEL_UP", &keyMap[KeyCodes::KEY_CHEAT_LEVEL_UP],
                       KeyDef{{.scancode = SDL_SCANCODE_4, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_CHEAT_LEVEL_UP",
                       "STR_OPTIONS_KEYBINDING");
    keys_.emplace_back("KEY_CHEAT_LEVEL_DOWN", &keyMap[KeyCodes::KEY_CHEAT_LEVEL_DOWN],
                       KeyDef{{.scancode = SDL_SCANCODE_5, .mod = KMOD_CTRL}}, "STR_OPTION_KEY_CHEAT_LEVEL_DOWN",
                       "STR_OPTIONS_KEYBINDING");
}

/**
 * Loads options from a YAML file.
 * @param filename YAML filename.
 * @return Was the loading successful?
 */
bool load(const std::string &filename) {
    std::string s = configFolder_ / filename;
    try {
        YAML::Node doc = YAML::LoadFile(s);
        for (auto &i : info_) {
            i.load(doc["options"]);
        }
        for (auto &i : keys_) {
            i.load(doc["keys"]);
        }
    } catch (YAML::Exception &e) {
        WARN(e.what());
        return false;
    }
    return true;
}

/**
 * Saves options to a YAML file.
 * @param filename YAML filename.
 * @return Was the saving successful?
 */
bool save(const std::string &filename) {
    std::string s = configFolder_ / filename;
    std::ofstream sav(s.c_str());
    if (!sav) {
        WARN("Failed to save %s", filename.c_str());
        return false;
    }
    try {
        YAML::Emitter out;

        YAML::Node doc, node_options, node_keys;
        for (auto &i : info_) {
            i.save(node_options);
        }
        for (auto &i : keys_) {
            i.save(node_keys);
        }
        doc["options"] = node_options;
        doc["keys"] = node_keys;

        writeNode(doc, out);

        sav << out.c_str() << std::endl;
    } catch (YAML::Exception &e) {
        WARN(e.what());
        return false;
    }
    sav.close();
    if (!sav) {
        WARN("Failed to save %s", filename.c_str());
        return false;
    }
    INFO("%s is saved", filename.c_str());
    return true;
}

/**
 * Resets the options back to their defaults.
 */
void resetDefault() {
    for (auto &i : info_) {
        i.reset();
    }
    for (auto &i : keys_) {
        i.reset();
    }
}

/**
 * Loads options from a set of command line arguments,
 * in the format "-option value".
 * @param argc Number of arguments.
 * @param argv Array of argument strings.
 */
void loadArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg[0] == '-' || arg[0] == '/') && arg.length() > 1) {
            std::string argname;
            if (arg[1] == '-' && arg.length() > 2)
                argname = arg.substr(2, arg.length() - 1);
            else
                argname = arg.substr(1, arg.length() - 1);
            std::transform(argname.begin(), argname.end(), argname.begin(), ::tolower);
            if (argc > i + 1) {
                ++i;

                // FIXME check argv[i]
                if (argname == "data") {
                    dataFolder_ = argv[i];
                } else if (argname == "user") {
                    userFolder_ = argv[i];
                } else if (argname == "config") {
                    configFolder_ = argv[i];
                } else {
                    // save this command line option for now, we will apply it later
                    commandLine_[argname] = argv[i];
                }
            } else {
                WARN("Unknown option: %s", argname.c_str());
            }
        }
    }
}

/**
 * Sets up the game's Data folder where the data files are loaded from and the
 * User folder and Config folder where settings and saves are stored in.
 */
void setFolders() {
    // FIXME move it to specific CrossPlatform class methods
    //_dataList = CrossPlatform::findDataFolders();
    dataList_.emplace_back(std::filesystem::path(SDL_GetPrefPath("Interrupt", "SystemShock")));

    if (!dataFolder_.empty()) {
        dataList_.insert(dataList_.begin(), dataFolder_);
    }
    if (userFolder_.empty()) {
        std::vector<std::string> user;
        user.emplace_back(std::string(SDL_GetPrefPath("Interrupt", "SystemShock")));
        // Look for an existing user folder
        for (auto const &i : user) {
            if (std::filesystem::is_directory(i)) {
                userFolder_ = i;
                break;
            }
        }

        if (configFolder_.empty()) {
            configFolder_ = std::string(SDL_GetPrefPath("Interrupt", "SystemShock"));
        }
    }
    if (!userFolder_.empty()) {
        std::vector<std::string> dirs = {"mods", "saves"};

        for (const auto &i : dirs) {
            std::filesystem::create_directories(userFolder_ / i);
        }
    }

    if (configFolder_.empty()) {
        configFolder_ = userFolder_;
    }
}

/*
 * Displays command-line help when appropriate.
 * @param argc Number of arguments.
 * @param argv Array of argument strings.
 */
bool showHelp(int argc, char *argv[]) {
    std::ostringstream help;
    help << SHOCK_VERSION << std::endl << std::endl;
    help << "Usage: " << argv[0] << " [OPTION]..." << std::endl << std::endl;
    help << "  --data PATH" << std::endl;
    help << "    use PATH as the default Data Folder instead of auto-detecting" << std::endl << std::endl;
    help << "  --user PATH" << std::endl;
    help << "    use PATH as the default User Folder instead of auto-detecting" << std::endl << std::endl;
    help << "  --config PATH" << std::endl;
    help << "    use PATH as the default Config Folder instead of auto-detecting" << std::endl << std::endl;
    help << "  --KEY VALUE" << std::endl;
    help << "    override option KEY with VALUE (eg. --displayWidth 640)" << std::endl << std::endl;
    help << "  --help" << std::endl;
    help << "    show command-line help" << std::endl;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg[0] == '-' && arg[1] == '-') && arg.length() > 2) {
            std::string argname = arg.substr(2, arg.length() - 2);

            std::transform(argname.begin(), argname.end(), argname.begin(), ::tolower);
            if (argname == "help") {
                std::cout << help.str();
                return true;
            }
        }
    }
    return false;
}

/**
 * Updates the game's options with those in the configuration
 * file, if it exists yet, and any supplied on the command line.
 */
void updateOptions() {
    // Load existing options
    if (std::filesystem::is_directory(configFolder_)) {
        if (std::filesystem::is_regular_file(configFolder_ / "options.cfg")) {
            load();
        } else {
            save();
        }
    } else {
        // Create config folder and save options
        std::filesystem::create_directory(configFolder_);
        save();
    }

    // now apply options set on the command line, overriding defaults and those loaded from config file
    for (auto &i : info_) {
        i.load(commandLine_);
    }
}

/**
 * Handles the initialization of setting up default options
 * and finding and loading any existing ones.
 * @param argc Number of arguments.
 * @param argv Array of argument strings.
 * @return Do we start the game?
 */
bool init(int argc, char *argv[]) {
    if (showHelp(argc, argv))
        return false;
    create();
    resetDefault();
    loadArgs(argc, argv);
    setFolders();
    updateOptions();

    log_set_level(Options::logLevel);
    // Save updated options
    save();

    std::string logfile = getUserFolder() / "shockplus.log";
    FILE *file = fopen(logfile.c_str(), "w");
    if (file) {
        log_add_fp(file, Options::logLevel);
    } else {
        WARN("Couldn't create log file, switching to stderr");
    }

    INFO(SHOCK_VERSION);
    INFO("Data folder is: %s", dataFolder_.c_str());
    INFO("Data search is:");
    for (auto &i : dataList_) {
        INFO("- %s", i.c_str());
    }
    INFO("User folder is: %s", userFolder_.c_str());
    INFO("Config folder is: %s", configFolder_.c_str());
    INFO("Options loaded successfully.");

    return true;
}

} // namespace ShockPlus::Options
