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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "helpers.h"

#include "yaml-cpp/yaml.h"

struct result_t {
    virtual void to_yaml(YAML::Emitter &out) = 0;
    virtual std::string get_index() = 0;
    virtual ~result_t() {}
};

template <typename TRAITS>
struct result : public result_t {
    using workload_conf_t = typename TRAITS::workload_conf_t;
    using parallel_conf_t = typename TRAITS::parallel_conf_t;
    using target_parameter_t = typename TRAITS::target_parameter_t;
    using workload_size_t = typename TRAITS::workload_size_t;
    using value_t = typename TRAITS::value_t;
    workload_conf_t workload_conf;
    parallel_conf_t parallel_conf;
    target_parameter_t target_parameter;
    workload_size_t workload_size;
    value_t value;
    std::string index;
    result() {}
    result(workload_conf_t _workload_conf, parallel_conf_t _parallel_conf, 
           target_parameter_t _target_parameter,
           workload_size_t _workload_size, double _value)
        : workload_conf(_workload_conf), parallel_conf(_parallel_conf), target_parameter(_target_parameter),
          workload_size(_workload_size), value(_value) {
        make_index();
    }

    void make_index() {
        std::stringstream ss;
        ss << TRAITS::workload_conf_to_string(workload_conf) << "_";
        ss << TRAITS::parallel_conf_to_string(parallel_conf) << "_";
        ss << TRAITS::target_parameter_to_string(target_parameter) << "_";
        ss << TRAITS::workload_size_to_string(workload_size);
        index = ss.str();
    }

    virtual void to_yaml(YAML::Emitter &out) {
        out << YAML::Flow;
        out << YAML::BeginMap;
        TRAITS::workload_conf_to_yaml(workload_conf, out);
        TRAITS::parallel_conf_to_yaml(parallel_conf, out);
        TRAITS::target_parameter_to_yaml(target_parameter, out);
        TRAITS::workload_size_to_yaml(workload_size, out);
        YAML_OUT("Value", value);
        out << YAML::EndMap;
    }
    virtual std::string get_index() { return index; }
    virtual ~result() {}
};
