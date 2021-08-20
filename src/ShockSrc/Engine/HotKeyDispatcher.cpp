/*

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

#include "HotKeyDispatcher.h"
#include "Options.h"

#include "audiolog.h"
#include "input.h"
#include "invent.h"
#include "hkeyfunc.h"
#include "hud.h"
#include "mfdfunc.h"
#include "mfdint.h"
#include "mainloop.h"
#include "olhext.h"
#include "wares.h"
#include "weapons.h"
#include "wrapper.h"

inline bool operator==(const SDL_Keysym &lhs, const SDL_Keysym &rhs) {
    return ((lhs.scancode + ((uint64_t)lhs.mod << 32u)) == (rhs.scancode + ((uint64_t)rhs.mod << 32u)));
}

inline bool operator>(const SDL_Keysym &lhs, const SDL_Keysym &rhs) {
    return ((lhs.scancode + ((uint64_t)lhs.mod << 32u)) > (rhs.scancode + ((uint64_t)rhs.mod << 32u)));
}

inline bool operator<(const SDL_Keysym &lhs, const SDL_Keysym &rhs) {
    return ((lhs.scancode + ((uint64_t)lhs.mod << 32u)) < (rhs.scancode + ((uint64_t)rhs.mod << 32u)));
}

namespace ShockPlus {

void HotKeyDispatcher::init() {
    hotkeys_ = {
        {Options::KeyCodes::KEY_NUMPAD_0,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 0}},
        {Options::KeyCodes::KEY_NUMPAD_1,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 1}},
        {Options::KeyCodes::KEY_NUMPAD_2,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 2}},
        {Options::KeyCodes::KEY_NUMPAD_3,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 3}},
        {Options::KeyCodes::KEY_NUMPAD_4,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 4}},
        {Options::KeyCodes::KEY_NUMPAD_5,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 5}},
        {Options::KeyCodes::KEY_NUMPAD_6,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 6}},
        {Options::KeyCodes::KEY_NUMPAD_7,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 7}},
        {Options::KeyCodes::KEY_NUMPAD_8,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 8}},
        {Options::KeyCodes::KEY_NUMPAD_9,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = keypad_hotkey_func, .state = 9}},
        {Options::KeyCodes::KEY_CANCEL_AUDIOLOG,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = audiolog_cancel_func, .state = 0}},
        {Options::KeyCodes::KEY_STAND,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = posture_hotkey_func, .state = 0}},
        {Options::KeyCodes::KEY_CROUCH,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = posture_hotkey_func, .state = 1}},
        {Options::KeyCodes::KEY_PRONE,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = posture_hotkey_func, .state = 2}},
        {Options::KeyCodes::KEY_TOGGLE_FREELOOK,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT | ShockPlus::Contexts::CYBER_CONTEXT,
                      .func = toggle_mouse_look,
                      .state = true}},
        {Options::KeyCodes::KEY_FULL_VIEW, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                        .func = change_mode_func,
                                                        .state = FULLSCREEN_LOOP}},

        {Options::KeyCodes::KEY_NORMAL_VIEW,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = change_mode_func, .state = GAME_LOOP}},
        {Options::KeyCodes::KEY_MAP_VIEW,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = change_mode_func, .state = AUTOMAP_LOOP}},
        {Options::KeyCodes::KEY_CLEAR_FULLSCREEN,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = clear_fullscreen_func, .state = 0}},
        {Options::KeyCodes::KEY_SAVE_GAME,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = saveload_hotkey_func, .state = false}},
        {Options::KeyCodes::KEY_LOAD_GAME,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = saveload_hotkey_func, .state = true}},

        {Options::KeyCodes::KEY_PAUSE,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = pause_game_func, .state = true}},

        {Options::KeyCodes::KEY_RELOAD_WEAPON_1,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = reload_weapon_hotkey, .state = 1}},
        {Options::KeyCodes::KEY_RELOAD_WEAPON_0,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = reload_weapon_hotkey, .state = 0}},
        {Options::KeyCodes::KEY_SELECT_GRENADE,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = select_grenade_hotkey, .state = 0}},
        {Options::KeyCodes::KEY_SELECT_DRUG,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = select_drug_hotkey, .state = 0}},
        {Options::KeyCodes::KEY_TOGGLE_OLH,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_olh_func, .state = 0}},

        {Options::KeyCodes::KEY_TOGGLE_MUSIC,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_music_func, .state = 0}},
        {Options::KeyCodes::KEY_QUIT,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = demo_quit_func, .state = 0}},
        {Options::KeyCodes::KEY_NEXT_WEAPON,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = cycle_weapons_func, .state = 1}},

        {Options::KeyCodes::KEY_PREV_WEAPON,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = cycle_weapons_func, .state = -1}},
        {Options::KeyCodes::KEY_CYCLE_DETAIL,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = MacDetailFunc, .state = 0}},
        {Options::KeyCodes::KEY_TOGGLE_OPENGL,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_opengl_func, .state = 0}},
        {Options::KeyCodes::KEY_ARM_GRENADE,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = arm_grenade_hotkey, .state = 0}},
        {Options::KeyCodes::KEY_USE_DRUG,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = use_drug_hotkey, .state = 0}},
        {Options::KeyCodes::KEY_HUD_COLOR,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = hud_color_bank_cycle, .state = 0}},
        {Options::KeyCodes::KEY_SHOW_HELP, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                        .func = olh_overlay_func,
                                                        .state = (intptr_t)&olh_overlay_on}},
        {Options::KeyCodes::KEY_BIOSCAN, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                      .func = hw_hotkey_callback,
                                                      .state = HARDWARE_BIOWARE}},
        {Options::KeyCodes::KEY_FULLSCREEN, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                         .func = hw_hotkey_callback,
                                                         .state = HARDWARE_FULLSCREEN}},
        {Options::KeyCodes::KEY_360_VIEW, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                       .func = hw_hotkey_callback,
                                                       .state = HARDWARE_360}},
        {Options::KeyCodes::KEY_LATERN, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                     .func = hw_hotkey_callback,
                                                     .state = HARDWARE_LANTERN}},
        {Options::KeyCodes::KEY_SHIELD, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                     .func = hw_hotkey_callback,
                                                     .state = HARDWARE_SHIELD}},
        {Options::KeyCodes::KEY_INFRARED, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                       .func = hw_hotkey_callback,
                                                       .state = HARDWARE_GOGGLE_INFRARED}},
        {Options::KeyCodes::KEY_NAV_UNIT, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                       .func = hw_hotkey_callback,
                                                       .state = HARDWARE_AUTOMAP}},
        {Options::KeyCodes::KEY_DATA_READER, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                          .func = hw_hotkey_callback,
                                                          .state = HARDWARE_EMAIL}},
        {Options::KeyCodes::KEY_BOOSTER, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                      .func = hw_hotkey_callback,
                                                      .state = HARDWARE_MOTION}},
        {Options::KeyCodes::KEY_JUMPJETS, HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT,
                                                       .func = hw_hotkey_callback,
                                                       .state = HARDWARE_SKATES}},
        {Options::KeyCodes::KEY_MFD_L1,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_L2,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_L3,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_L4,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_L5,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_R1,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_R2,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_R3,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_R4,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_MFD_R5,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = mfd_button_callback_kb, .state = 0}},
        {Options::KeyCodes::KEY_TOGGLE_OPTIONS,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = wrapper_options_func, .state = 0}},
        {Options::KeyCodes::KEY_CHEAT_GIVEALL,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_giveall_func, .state = true}},
        {Options::KeyCodes::KEY_CHEAT_PHYSICS,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_physics_func, .state = true}},
        {Options::KeyCodes::KEY_CHEAT_LEVEL_UP,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_up_level_func, .state = true}},
        {Options::KeyCodes::KEY_CHEAT_LEVEL_DOWN,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_down_level_func, .state = true}},
        {Options::KeyCodes::KEY_CHEAT_LEVEL_DOWN,
         HotkeyLookup{.contexts = ShockPlus::Contexts::DEMO_CONTEXT, .func = toggle_down_level_func, .state = true}}};

    for (uint32_t i = 0; i < Options::KeyCodes::KEY_COUNT; i++) {
        for (auto const &j : Options::keyMap[i]) {
            key_actions_.insert({j, i});
        }
    }
}

bool HotKeyDispatcher::hotkey_dispatch(SDL_Keysym keysym) {
    keysym.mod &= mod_filter_;
    auto range = key_actions_.equal_range(keysym);
    if (!(range.first == key_actions_.end() && range.second == key_actions_.end())) {
        for (auto i = range.first; i != range.second; i++) {
            if (hotkeys_.find(i->second) != hotkeys_.end()) {
                if (hotkeys_[i->second].contexts & hotkey_context_) {
                    if (hotkeys_[i->second].func(i->second, hotkey_context_, hotkeys_[i->second].state)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

} // namespace ShockPlus
