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

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <cmath>

#include "argsparser.h"
#include "helpers.h"
#include "scope.h"
#include "modules/imb_async/traits.h"
#include "modules/imb_async/inout.h"
#include "results.h"

namespace imb_async {

std::vector<traits::target_parameter_t>
traits::parse_and_make_target_parameters(const args_parser &parser, const std::string &name) {
    return helpers::parsers_vector_to_vector<std::string, std::string>(parser, name);
}

std::vector<traits::parallel_conf_t>
traits::parse_and_make_parallel_confs(const args_parser &parser, const std::string &name) {
    return helpers::parsers_vector_to_vector<int, int>(parser, name);
}

std::vector<traits::workload_size_t>
traits::parse_and_make_workload_sizes(const args_parser &parser, const std::string &name) {
    return helpers::parsers_map_to_vector<size_t, int>(parser, name);
}

std::shared_ptr<test_scope<traits>>
traits::make_scope(const std::vector<traits::parallel_conf_t> &parallel_confs,
                   const std::vector<traits::target_parameter_t> &target_parameters,
                   const std::vector<traits::workload_size_t> workload_sizes) {
    return std::make_shared<test_scope<imb_async::traits>>(parallel_confs, target_parameters,
                                                           workload_sizes);
}

std::shared_ptr<input_maker_base> traits::make_input_maker(test_scope<traits> &scope) {
    return std::make_shared<input_maker>(scope);
}

std::shared_ptr<output_maker_base> traits::make_output_maker(test_scope<traits> &scope,
                                                             const std::string &outfile) {
    return std::make_shared<output_maker>(scope, outfile);
}

std::shared_ptr<result_t> traits::make_result(const parallel_conf_t &pc,
                                              const target_parameter_t &tp,
                                              const workload_size_t &ws, double value) {
    return std::make_shared<result<traits>>(pc, tp, ws, value);
}

} // namespace imb_async

std::ostream &operator<<(std::ostream &out,
                         const typename imb_async::traits::parallel_conf_t conf) {
    out << conf.first << "+" << conf.second;
    return out;
}

std::ostream &operator<<(std::ostream &out,
                         const typename imb_async::traits::target_parameter_t par) {
    out << par.first << "+" << par.second;
    return out;
}

std::ostream &operator<<(std::ostream &out,
                         const typename imb_async::traits::workload_size_t work) {
    out << work.first << "+" << work.second;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const typename imb_async::traits::parallel_conf_t conf) {
    out << YAML::Key << YAML::Flow << "n" << YAML::Value << conf.first;
    out << YAML::Key << YAML::Flow << "ppn" << YAML::Value << conf.second;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const typename imb_async::traits::target_parameter_t par) {
    out << YAML::Key << YAML::Flow << "Benchmark" << YAML::Value << par.first;
    out << YAML::Key << YAML::Flow << "Parameter" << YAML::Value << par.second;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const typename imb_async::traits::workload_size_t work) {
    out << YAML::Key << YAML::Flow << "Length" << YAML::Value << work.first;
    out << YAML::Key << YAML::Flow << "Iter" << YAML::Value << work.second;
    return out;
}
