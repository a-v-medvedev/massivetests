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

#pragma once

template <typename parallel_conf_t>
class process;

class execution_environment;

template <typename parallel_conf_t>
struct input_maker_base {
    std::string preproc, postproc;
    virtual bool make(const parallel_conf_t &pconf, execution_environment &env) = 0;
    virtual ~input_maker_base() {}
};

template <typename parallel_conf_t>
struct output_maker_base {
    std::string preproc, postproc;
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts) = 0;
    virtual ~output_maker_base() {}
};
