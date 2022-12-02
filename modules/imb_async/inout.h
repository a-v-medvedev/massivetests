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

#include <vector>
#include <string>
#include <memory>

#include "modules/imb_async/traits.h"
#include "scope.h"
#include "process.h"

namespace imb_async {

template <typename parallel_conf_t>
struct input_maker : public input_maker_base<parallel_conf_t> {
    bool was_written;
    test_scope<traits> &scope;
    input_maker(test_scope<traits> &_scope);
    void write_out(const std::string &input_file_name);
    virtual void make(const parallel_conf_t &pconf, execution_environment &env) override;
};

template <typename parallel_conf_t>
struct output_maker : public output_maker_base<parallel_conf_t> {
    test_scope<traits> scope;
    YAML::Emitter out;
    std::string outfile;
    output_maker(test_scope<traits> &_scope, const std::string &_outfile);
    ~output_maker();
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts);
};

} // namespace imb_async
