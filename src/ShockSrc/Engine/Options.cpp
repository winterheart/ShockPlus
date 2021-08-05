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

std::string dataFolder_;
std::vector<std::string> dataList_;
std::string userFolder_;
std::string configFolder_;
std::vector<std::string> userList_;
std::map<std::string, std::string> commandLine_;
std::vector<OptionInfo> info_;

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
std::string getConfigFolder() { return configFolder_; }

/**
 * Returns the game's current Data folder where resources are loaded from.
 * @return Full path to Data folder.
 */
std::string getDataFolder() { return dataFolder_; }

/**
 * Returns the game's User folder where saves are stored in.
 * @return Full path to User folder.
 */
std::string getUserFolder() { return userFolder_; }

/**
 * Returns the game's list of all available option information.
 * @return List of OptionInfo's.
 */
const std::vector<OptionInfo> &getOptionInfo() { return info_; }

/**
 * Returns the game's list of possible Data folders.
 * @return List of Data paths.
 */
const std::vector<std::string> &getDataList() { return dataList_; }

/**
 * Changes the game's current Data folder where resources are loaded from.
 * @param folder Full path to Data folder.
 */
void setDataFolder(const std::string &folder) { dataFolder_ = folder; }

/**
 * Sets up the options by creating their OptionInfo metadata.
 */
void create() {
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
}

/**
 * Loads options from a YAML file.
 * @param filename YAML filename.
 * @return Was the loading successful?
 */
bool load(const std::string &filename) {
    std::string s = configFolder_ + filename;
    try {
        YAML::Node doc = YAML::LoadFile(s);
        for (auto &i : info_) {
            i.load(doc["options"]);
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
    std::string s = configFolder_ + filename;
    std::ofstream sav(s.c_str());
    if (!sav) {
        WARN("Failed to save %s", filename.c_str());
        return false;
    }
    try {
        YAML::Emitter out;

        YAML::Node doc, node;
        for (auto &i : info_) {
            i.save(node);
        }
        doc["options"] = node;

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
    // backupDisplay();
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
    dataList_.emplace_back(std::string(SDL_GetPrefPath("Interrupt", "SystemShock")));

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
        std::filesystem::path modDir(userFolder_ + "/mods");
        // create mod folder if it doesn't already exist
        if (!std::filesystem::exists(modDir)) {
            std::filesystem::create_directory(modDir);
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
        if (std::filesystem::is_regular_file(configFolder_ + "options.cfg")) {
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
    save("options.cfg");

    std::string logfile = getUserFolder() + "shockplus.log";
    FILE *file = fopen(logfile.c_str(), "w");
    if (file) {
        log_add_fp(file, Options::logLevel);
    } else {
        WARN("Couldn't create log file, switching to stderr");
    }

    INFO(SHOCK_VERSION);
#ifdef _WIN32
    INFO("Platform: Windows");
#elif __APPLE__
    INFO("Platform: OSX");
#elif __ANDROID_API__
    INFO("Platform: Android");
#else
    INFO("Platform: Unix-like");
#endif

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
