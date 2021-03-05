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
#include "helpers.h"

class result_t;

namespace teststub {

struct traits {
    using parallel_conf_t = std::pair<int, int>;
    using target_parameter_t = std::pair<std::string, std::string>;
    struct workload_size_t { 
        std::string workload; 
        std::string matrix;
        bool operator<(const workload_size_t &other) const { return other.workload < workload && other.matrix < matrix; }
        std::string as_string() const { 
	        std::stringstream ss;
    	    ss << workload << "+" << matrix;
        	return ss.str();
        }
    };
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
    static std::string parallel_conf_to_string(const parallel_conf_t &c) {
        std::stringstream ss;
        ss << c.first << "+" << c.second;
        return ss.str();
    }
    static std::string target_parameter_to_string(const target_parameter_t &tp) {
        std::stringstream ss;
        ss << tp.first << "+" << tp.second;
        return ss.str();
    }
    static std::string workload_size_to_string(const workload_size_t &ws) {
		return ws.as_string();
    }
    static void parallel_conf_to_yaml(const parallel_conf_t &c, YAML::Emitter &out) {
        out << YAML::Key << YAML::Flow << "n" << YAML::Value << c.first;
        out << YAML::Key << YAML::Flow << "ppn" << YAML::Value << c.second;
    }
    static void target_parameter_to_yaml(const target_parameter_t &tp, YAML::Emitter &out) {
        out << YAML::Key << YAML::Flow << "Section" << YAML::Value << tp.first.c_str();
        out << YAML::Key << YAML::Flow << "Parameter" << YAML::Value << tp.second.c_str();
    }
    static void workload_size_to_yaml(const workload_size_t &ws, YAML::Emitter &out) {
        out << YAML::Key << YAML::Flow << "Workload" << YAML::Value << ws.workload;
        out << YAML::Key << YAML::Flow << "Matrix" << YAML::Value << ws.matrix;
    }

};

} // namespace teststub


