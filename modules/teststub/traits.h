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

#include <string>
#include <vector>
#include <utility>
#include <memory>

#include "argsparser.h"
#include "scope.h"
#include "inout_base.h"

class result_t;

namespace teststub {

struct traits {
    using parallel_conf_t = std::pair<int, int>;
    using target_parameter_t = std::pair<std::string, std::string>;
    using workload_size_t = std::string;
    using value_t = int;
    std::vector<target_parameter_t> parse_and_make_target_parameters(const args_parser &parser,
                                                                     const std::string &name);
    std::vector<parallel_conf_t> parse_and_make_parallel_confs(const args_parser &parser,
                                                               const std::string &name);
    std::vector<workload_size_t> parse_and_make_workload_sizes(const args_parser &parser,
                                                               const std::string &name);
    std::shared_ptr<test_scope<traits>>
    make_scope(const std::vector<traits::parallel_conf_t> &parallel_confs,
               const std::vector<traits::target_parameter_t> &target_parameters,
               const std::vector<traits::workload_size_t> workload_sizes);
    std::shared_ptr<input_maker_base> make_input_maker(test_scope<traits> &scope);
    std::shared_ptr<output_maker_base> make_output_maker(test_scope<traits> &scope,
                                                         const std::string &outfile);
    std::shared_ptr<result_t> make_result(const parallel_conf_t &pc, const target_parameter_t &tp,
                                          const workload_size_t &ws, value_t status);
};

} // namespace teststub

std::ostream &operator<<(std::ostream &out, const typename teststub::traits::parallel_conf_t conf);
std::ostream &operator<<(std::ostream &out,
                         const typename teststub::traits::target_parameter_t par);
std::ostream &operator<<(std::ostream &out, const typename teststub::traits::workload_size_t work);
YAML::Emitter &operator<<(YAML::Emitter &out,
                          const typename teststub::traits::parallel_conf_t conf);
YAML::Emitter &operator<<(YAML::Emitter &out,
                          const typename teststub::traits::target_parameter_t par);
YAML::Emitter &operator<<(YAML::Emitter &out,
                          const typename teststub::traits::workload_size_t work);
