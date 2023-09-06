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
#include <fstream>

namespace functest {
enum status_t { P=0, F=1, N=2, S=3, T=4, A=5, C=6, E=7 };
}

#include "modules/functest/traits.h"
#include "scope.h"
#include "process.h"
#include "testitem.h"

namespace functest {

template <typename parallel_conf_t>
struct input_maker : public input_maker_base<parallel_conf_t> {
    std::string load_key = "-load";
    std::string result_key = "-result";
    std::string conf_key = "";
    std::string timeout_key = "-timeout";
    test_item_t testitem;
    test_scope<traits> &scope;
    input_maker(test_scope<traits> &_scope);
    bool check_prerequisites(const parallel_conf_t &pconf, execution_environment &env);
    virtual bool make(const parallel_conf_t &pconf, execution_environment &env);
	void do_substs(const parallel_conf_t &pconf, std::string &filename);
	bool file_exists(const parallel_conf_t &pconf, const std::string &filename_);
	bool exec_shell_command(const parallel_conf_t &pconf, const test_item_t &testitem,
                            const std::string &script, const std::vector<std::string> &exports, 
                            std::string &result, int &status);
};

template <typename parallel_conf_t>
struct output_maker : public output_maker_base<parallel_conf_t> {
    test_item_t testitem;
    test_scope<traits> &scope;
    YAML::Emitter out;
    std::string outfile;
    output_maker(test_scope<traits> &_scope, const std::string &_outfile);
    virtual ~output_maker();
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts);
};

} // namespace functest
