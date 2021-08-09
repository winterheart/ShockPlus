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

namespace ShockPlus {

bool HotKeyDispatcher::hotkey_dispatch(uint16_t keycode) {
    auto range = hotkeys_.equal_range(keycode);
    if (!(range.first == hotkeys_.end() && range.second == hotkeys_.end())) {
        for (auto i = range.first; i != range.second; ++i) {
            if ((i->second.contexts & hotkey_context_) && i->second.func(keycode, hotkey_context_, i->second.state)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace ShockPlus
