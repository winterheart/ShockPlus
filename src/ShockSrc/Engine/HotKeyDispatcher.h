#pragma once
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

#include <functional>
#include <map>
#include <string>

namespace ShockPlus {

enum Contexts : uint32_t {
    DEMO_CONTEXT = 0x01,       // main context
    EDIT_CONTEXT = 0x02,       // unused
    CYBER_CONTEXT = 0x04,      // cyber context
    SETUP_CONTEXT = 0x08,      // setup context
    MWORK_CONTEXT = 0x10,      // unused
    SVGA_CONTEXT = 0x20,       // unused
    AMAP_CONTEXT = 0x40,       // automap context
    EVERY_CONTEXT = 0xFFFFFFFF // all context
};

typedef struct hotkeylookup_struct {
    intptr_t contexts;                                                               // contexts that applies hotkey
    std::function<uint8_t(uint16_t keycode, uint32_t context, intptr_t state)> func; // callback function
    intptr_t state;                                                                  // state for callback
} HotkeyLookup;

class HotKeyDispatcher {
  private:
    /// All hotkeys map
    std::multimap<uint16_t, HotkeyLookup> hotkeys_;
    /// Hotkey context that applies all matched hotkeys
    uint32_t hotkey_context_ = Contexts::EVERY_CONTEXT;

  public:
    /**
     * Add hotkey to map of dispatcher
     * @param keycode key
     * @param lookup hotkey lookup that contains contexts, callback and state
     */
    void add(uint16_t keycode, const HotkeyLookup &lookup) { hotkeys_.insert(std::make_pair(keycode, lookup)); };
    /**
     * Remove hotkey from map of dispatcher
     * @param keycode key
     */
    void remove(uint16_t keycode) { hotkeys_.erase(keycode); }

    /**
     * Emit hotkey event to function callback that assigned to keycode
     * @param keycode key
     * @return true on success, false if there no such keycode
     */
    bool hotkey_dispatch(uint16_t keycode);

    /**
     * Get current hotkey context
     * @return current hotkey context
     */
    [[nodiscard]] uint32_t getContext() const { return hotkey_context_; }
    /**
     * Set hotkey context
     * @param context new context value
     */
    void setContext(uint32_t context) { hotkey_context_ = context; }
};

} // namespace ShockPlus
