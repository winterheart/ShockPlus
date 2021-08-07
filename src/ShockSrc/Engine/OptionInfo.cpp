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

#include "Exception/Exception.h"
#include "OptionInfo.h"

namespace ShockPlus {

/**
 * Creates info for a boolean option.
 * @param id String ID used in serializing.
 * @param option Pointer to the option.
 * @param def Default option value.
 * @param desc Language ID for the option description (if any).
 * @param cat Language ID for the option category (if any).
 */
OptionInfo::OptionInfo(std::string id, bool *option, bool def, std::string desc, std::string cat)
    : id_(std::move(id)), desc_(std::move(desc)), cat_(std::move(cat)), type_(OPTION_BOOL) {
    ref_ = {.b = option};
    def_ = {.b = def};
}

/**
 * Creates info for an integer option.
 * @param id String ID used in serializing.
 * @param option Pointer to the option.
 * @param def Default option value.
 * @param desc Language ID for the option description (if any).
 * @param cat Language ID for the option category (if any).
 */
OptionInfo::OptionInfo(std::string id, int *option, int def, std::string desc, std::string cat)
    : id_(std::move(id)), desc_(std::move(desc)), cat_(std::move(cat)), type_(OPTION_INT) {
    ref_ = {.i = option};
    def_ = {.i = def};
}

/**
 * Creates info for a string option.
 * @param id String ID used in serializing.
 * @param option Pointer to the option.
 * @param def Default option value.
 * @param desc Language ID for the option description (if any).
 * @param cat Language ID for the option category (if any).
 */
OptionInfo::OptionInfo(std::string id, std::string *option, const char *def, std::string desc, std::string cat)
    : id_(std::move(id)), desc_(std::move(desc)), cat_(std::move(cat)), type_(OPTION_STRING) {
    ref_ = {.s = option};
    def_ = {.s = def};
}

/**
 * Returns the pointer to the boolean option.
 * @throws if option is not a boolean
 * @return Pointer to the option.
 */
bool *OptionInfo::asBool() const {
    if (type_ != OPTION_BOOL) {
        throw Exception(id_ + " is not a boolean!");
    }
    return ref_.b;
}

/**
 * Returns the pointer to the integer option,
 * @throws if option is not a integer.
 * @return Pointer to the option.
 */
int *OptionInfo::asInt() const {
    if (type_ != OPTION_INT) {
        throw Exception(id_ + " is not a integer!");
    }
    return ref_.i;
}

/**
 * Returns the pointer to the string option,
 * @throws if option is not a string.
 * @return Pointer to the option.
 */
std::string *OptionInfo::asString() const {
    if (type_ != OPTION_STRING) {
        throw Exception(id_ + " is not a string!");
    }
    return ref_.s;
}

/**
 * Loads an option value from the corresponding YAML.
 * @param node Options YAML node.
 */
void OptionInfo::load(const YAML::Node &node) const {
    switch (type_) {
    case OPTION_BOOL:
        *(ref_.b) = node[id_].as<bool>(def_.b);
        break;
    case OPTION_INT:
        *(ref_.i) = node[id_].as<int>(def_.i);
        break;
        /*
        case OPTION_KEY:
            *(_ref.k) = (SDLKey)node[_id].as<int>(_def.k);
            break;
        */
    case OPTION_STRING:
        *(ref_.s) = node[id_].as<std::string>(def_.s);
        break;
    }
}

/**
 * Loads an option value from the corresponding map (eg. for command-line options).
 * @param map Options map.
 */
void OptionInfo::load(const std::map<std::string, std::string> &map) const {
    std::string id = id_;
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    // std::map<std::string, std::string>::const_iterator it = map.find(id);
    auto it = map.find(id);
    if (it != map.end()) {
        std::string value = it->second;
        std::stringstream ss;
        switch (type_) {
        case OPTION_BOOL:
            bool b;
            ss << std::boolalpha << value;
            ss >> std::boolalpha >> b;
            *(ref_.b) = b;
            break;
        case OPTION_INT:
            int i;
            ss << std::dec << value;
            ss >> std::dec >> i;
            *(ref_.i) = i;
            break;
            /*
            case OPTION_KEY:
                ss << std::dec << value;
                ss >> std::dec >> i;
                *(_ref.k) = (SDLKey)i;
                break;
            */
        case OPTION_STRING:
            *(ref_.s) = value;
            break;
        }
    }
}

/**
 * Saves an option value to the corresponding YAML.
 * @param node Options YAML node.
 */
void OptionInfo::save(YAML::Node &node) const {
    switch (type_) {
    case OPTION_BOOL:
        node[id_] = *(ref_.b);
        break;
    case OPTION_INT:
        node[id_] = *(ref_.i);
        break;
    /*
    case OPTION_KEY:
        node[_id] = (int)*(_ref.k);
        break;
    */
    case OPTION_STRING:
        node[id_] = *(ref_.s);
        break;
    }
}

/**
 * Resets an option back to its default value.
 */
void OptionInfo::reset() const {
    switch (type_) {
    case OPTION_BOOL:
        *(ref_.b) = def_.b;
        break;
    case OPTION_INT:
        *(ref_.i) = def_.i;
        break;
    /*
    case OPTION_KEY:
        *(_ref.k) = _def.k;
        break;
    */
    case OPTION_STRING:
        *(ref_.s) = def_.s;
        break;
    }
}

/**
 * Returns the variable type of the option.
 * @return Option type.
 */
OptionType OptionInfo::type() const { return type_; }

/**
 * Returns the description of the option. Options with
 * descriptions show up in the Options screens.
 * @return Language string ID for the description.
 */
std::string OptionInfo::description() const { return desc_; }

/**
 * Returns the category of the option. Options with
 * categories show up in the Options screens.
 * @return Language string ID for the category.
 */
std::string OptionInfo::category() const { return cat_; }

} // namespace ShockPlus
