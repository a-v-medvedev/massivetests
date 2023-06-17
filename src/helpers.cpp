/*
    This file is part of massivetest.

    Massivetest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    massivetests is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with massivetests.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "helpers.h"

namespace helpers {

template <>
std::string str2value<std::string>(const std::string &value) {
    return value;
}

template <>
int str2value<int>(const std::string &value) {
    return std::stoi(value);
}

template <>
size_t str2value<size_t>(const std::string &value) {
    return std::stoul(value);
}

template <>
double str2value<double>(const std::string &value) {
    return std::stod(value);
}

template <>
bool str2value<bool>(const std::string &value) {
    return value == "true";
}

template <>
std::string value2str<double>(const double &x) { return helpers::flt2str(x); }

template <>
std::string value2str<int>(const int &x) { return std::to_string(x); }

template <>
std::string value2str<bool>(const bool &x) { return bool2str(x); }

template <>
std::string value2str<std::string>(const std::string &x) { return x; }

} // namespace helpers
