/*
    This file is part of massivetest.

    Massivetest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

class process;

struct input_maker_base {
    virtual void make(std::string &input_yaml, std::string &psubmit_options, std::string &args) = 0;
    virtual ~input_maker_base() {}
};

struct output_maker_base {
    virtual void make(std::vector<std::shared_ptr<process>> &attempts) = 0;
    virtual ~output_maker_base() {}
};
