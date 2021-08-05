/*

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

// To add a new option, add a new variable entry and a corresponding OptionInfo in Options.cpp

// Non-UI options
OPT LogLevel logLevel;

// Control options
OPT bool captureMouse, invertMouseY;

// General options
OPT bool showIntro, showOnScreenHelp, showSplash, showTooltipMessages;
// OPT std::string language;
OPT GeneralLanguage language;
OPT MessageFormat messageFormat;

// Sound options
OPT bool enableSFX, enableSound;
OPT int midiOutput, musicVolume, sfxVolume, voiceVolume;
OPT AlogPlayback alogPlayback;
OPT MIDIBackend midiBackend;

// Video options
OPT bool enableOpenGL, halfResolution, enableSkipLines;
OPT int videoMode, gammaCorrection;
OPT VideoDetail videoDetail;
OPT VideoTextureFilter videoTextureFilter;
