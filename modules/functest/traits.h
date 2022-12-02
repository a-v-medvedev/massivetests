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

#include <string>
#include <vector>
#include <utility>
#include <memory>

#include "argsparser.h"
#include "scope.h"
#include "inout_base.h"
#include "helpers.h"

class result_t;

namespace functest {

struct traits {
    using workload_conf_t = std::pair<std::string, std::string>;
    using parallel_conf_t = std::pair<int, int>;
    using target_parameter_t = std::pair<std::string, std::string>;
    using workpart_t = std::pair<std::string, int>;
    using value_t = std::string;
    static std::string application;
    std::vector<workload_conf_t> parse_and_make_workload_confs(const args_parser &parser,
                                                               const std::string &name);
    std::vector<target_parameter_t> parse_and_make_target_parameters(const args_parser &parser,
                                                                     const std::string &name);
    std::vector<parallel_conf_t> parse_and_make_parallel_confs(const args_parser &parser,
                                                               const std::string &name);
    std::vector<workpart_t> parse_and_make_workparts(const args_parser &parser,
                                                               const std::string &name);
    std::shared_ptr<test_scope<traits>>
    make_scope(const workload_conf_t &c,
               const std::vector<traits::parallel_conf_t> &parallel_confs,
               const std::vector<traits::target_parameter_t> &target_parameters,
               const std::vector<traits::workpart_t> workparts);
	std::vector<std::shared_ptr<test_scope<traits>>>
	make_scopes(const std::vector<traits::workload_conf_t> &workload_confs,
                const std::vector<traits::parallel_conf_t> &parallel_confs,
                const std::vector<traits::target_parameter_t> &target_parameters,
                const std::vector<traits::workpart_t> workparts);
    std::shared_ptr<input_maker_base<parallel_conf_t>> make_input_maker(test_scope<traits> &scope);
    std::shared_ptr<output_maker_base<parallel_conf_t>> make_output_maker(test_scope<traits> &scope,
                                                         const std::string &outfile);
    std::shared_ptr<result_t> make_result(const workload_conf_t &c, 
                                          const parallel_conf_t &pc, 
                                          const target_parameter_t &tp,
                                          const workpart_t &ws, 
                                          value_t status,
                                          const std::string &comment = "");
    static std::string workload_conf_to_string(const workload_conf_t &c) {
        if (c.first == "" && c.second == "")
            return "";
        std::stringstream ss;
        ss << c.first << "/" << c.second;
        return ss.str();
    }
    static std::string parallel_conf_to_string(const parallel_conf_t &c) {
        std::stringstream ss;
        ss << c.first << "/" << c.second;
        return ss.str();
    }
    static std::string target_parameter_to_string(const target_parameter_t &tp) {
        if (tp.first == "" && tp.second == "")
            return "";
        std::stringstream ss;
        ss << tp.first << "/" << tp.second;
        return ss.str();
    }
    static std::string workpart_to_string(const workpart_t &ws) {
        if (ws.first == "")
            return "";
        std::stringstream ss;
        ss << ws.first << "/" << ws.second;
        return ss.str();
    }
    static void workload_conf_to_yaml(const workload_conf_t &c, YAML::Emitter &out) {
        if (c.first == "" && c.second == "")
            return;
        out << YAML::Key << YAML::Flow << "Workload" << YAML::Value << c.first;
        out << YAML::Key << YAML::Flow << "Mode" << YAML::Value << c.second;
    }
    static void parallel_conf_to_yaml(const parallel_conf_t &c, YAML::Emitter &out) {
        out << YAML::Key << YAML::Flow << "n" << YAML::Value << c.first;
        out << YAML::Key << YAML::Flow << "ppn" << YAML::Value << c.second;
    }
    static void target_parameter_to_yaml(const target_parameter_t &tp, YAML::Emitter &out) {
        if (tp.first == "" && tp.second == "")
            return;
        out << YAML::Key << YAML::Flow << "Section" << YAML::Value << tp.first;
        out << YAML::Key << YAML::Flow << "Parameter" << YAML::Value << tp.second;
    }
    static void workpart_to_yaml(const workpart_t &ws, YAML::Emitter &out) {
        if (ws.first == "")
            return;
        out << YAML::Key << YAML::Flow << "Workpart" << YAML::Value << ws.first;
        out << YAML::Key << YAML::Flow << "Iters" << YAML::Value << ws.second;
    }
};

} // namespace functest


