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

#include <yaml-cpp/yaml.h>
#include "argsparser.h"
#include "helpers.h"
#include "scope.h"
#include "modules/functest/traits.h"
#include "modules/functest/inout.h"
#include "modules/functest/inout_teststub.h"
#include "modules/functest/inout_xamg.h"
#include "modules/functest/inout_qubiq.h"
#include "modules/functest/inout_fs1D.h"
#include "results.h"

namespace functest {

std::vector<traits::workload_conf_t> parse_and_make_workload_confs(const args_parser &parser,
														   const std::string &name) {
	return helpers::parsers_vector_to_vector<std::string, std::string>(parser, name);
}
    
std::vector<traits::target_parameter_t>
traits::parse_and_make_target_parameters(const args_parser &parser, const std::string &name) {
    return helpers::parsers_vector_to_vector<std::string, std::string>(parser, name);
}

std::vector<traits::parallel_conf_t>
traits::parse_and_make_parallel_confs(const args_parser &parser, const std::string &name) {
    return helpers::parsers_vector_to_vector<int, int>(parser, name);
}

std::vector<traits::workpart_t>
traits::parse_and_make_workparts(const args_parser &parser, const std::string &name) {
    return helpers::parsers_vector_to_vector<std::string, int>(parser, name);
}

std::shared_ptr<test_scope<traits>>
traits::make_scope(const traits::workload_conf_t &workload_conf,
                   const std::vector<traits::parallel_conf_t> &parallel_confs,
                   const std::vector<traits::target_parameter_t> &target_parameters,
                   const std::vector<traits::workpart_t> workparts) {
    return std::make_shared<test_scope<functest::traits>>(workload_conf, 
                                                          parallel_confs, 
                                                          target_parameters,
                                                          workparts);
}

std::vector<std::shared_ptr<test_scope<traits>>>
traits::make_scopes(const std::vector<traits::workload_conf_t> &workload_confs,
                    const std::vector<traits::parallel_conf_t> &parallel_confs,
                    const std::vector<traits::target_parameter_t> &target_parameters,
                    const std::vector<traits::workpart_t> workparts) {
    std::vector<std::shared_ptr<test_scope<traits>>> vec;
    for (const auto &w : workload_confs) {
        for (const auto &s : workparts) {
            vec.push_back(make_scope(w, parallel_confs, target_parameters, {s}));
        }
    }
    return vec;
}

std::shared_ptr<input_maker_base<traits::parallel_conf_t>> 
traits::make_input_maker(test_scope<traits> &scope) {
    if (application == "teststub") {
        return std::make_shared<input_maker_teststub<traits::parallel_conf_t>>(scope);
    } else if (application == "xamg") {
        return std::make_shared<input_maker_xamg<traits::parallel_conf_t>>(scope);
    } else if (application == "qubiq") {
        return std::make_shared<input_maker_qubiq<traits::parallel_conf_t>>(scope);
    } else if (application == "fs1D") {
        return std::make_shared<input_maker_fs1D<traits::parallel_conf_t>>(scope);
    } else {
        throw std::runtime_error("input_maker: Unknown application");
        return nullptr;
    }
}

std::shared_ptr<output_maker_base<traits::parallel_conf_t>> 
traits::make_output_maker(test_scope<traits> &scope, const std::string &outfile) {
    if (application == "teststub") {
        return std::make_shared<output_maker_teststub<traits::parallel_conf_t>>(scope, outfile);
    } else if (application == "xamg") {
        return std::make_shared<output_maker_xamg<traits::parallel_conf_t>>(scope, outfile);
    } else if (application == "qubiq") {
        return std::make_shared<output_maker_qubiq<traits::parallel_conf_t>>(scope, outfile);
    } else if (application == "fs1D") {
        return std::make_shared<output_maker_fs1D<traits::parallel_conf_t>>(scope, outfile);
    } else {
        throw std::runtime_error("output_maker: Unknown application");
        return nullptr;
    }
}

std::shared_ptr<result_t> traits::make_result(const workload_conf_t &wc, 
                                              const parallel_conf_t &pc,
                                              const target_parameter_t &tp,
                                              const workpart_t &ws, 
                                              value_t value,
                                              const std::string &comment) {
    return std::make_shared<result<traits>>(wc, pc, tp, ws, value, comment);
}

std::string traits::application;

} // namespace functest

