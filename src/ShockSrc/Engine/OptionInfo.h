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
#include <variant>
#include <yaml-cpp/yaml.h>

namespace ShockPlus {

enum OptionType { OPTION_BOOL, OPTION_INT, OPTION_STRING };

class OptionInfo {
  private:
    std::string id_, desc_, cat_;
    OptionType type_;

    std::variant<bool *, int *, std::string *> ref_;
    std::variant<bool, int, std::string> def_;

  public:
    /// Creates a bool option
    OptionInfo(std::string id, bool *option, bool def, std::string desc = "", std::string cat = "");
    /// Creates a int option.
    OptionInfo(std::string id, int *option, int def, std::string desc = "", std::string cat = "");
    /// Creates a string option.
    OptionInfo(std::string id, std::string *option, std::string def, std::string desc = "", std::string cat = "");

    /// Gets a bool option pointer.
    bool *asBool() const;
    /// Gets an int option pointer.
    int *asInt() const;
    /// Gets a string option pointer.
    std::string *asString() const;

    void load(const YAML::Node &node) const;
    /// Loads the option from a map.
    void load(const std::map<std::string, std::string> &map) const;
    /// Saves the option to YAML.
    void save(YAML::Node &node) const;
    /// Resets the option to default.
    void reset() const;
    /// Gets the option type.
    OptionType type() const;
    /// Gets the option description.
    std::string description() const;
    /// Gets the option category.
    std::string category() const;
};

} // namespace ShockPlus
