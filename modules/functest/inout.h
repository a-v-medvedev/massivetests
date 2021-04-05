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

#include <vector>
#include <string>
#include <memory>
#include <fstream>

#include "modules/functest/traits.h"
#include "scope.h"
#include "process.h"

namespace functest {

struct test_item_t {
    std::string name;
    std::map<std::string, double> base;
    bool skip = false;
    unsigned timeout = 1000;
    void load(const std::string &_name) {
        name = _name;
        std::ifstream in;
        in.open("test_items.yaml");
        auto stream = YAML::Load(in);
        if (!stream[name]) {
            skip = true;
            return;
        }
        const auto &item = stream[name].as<YAML::Node>();
        const auto &opts = item["options"].as<YAML::Node>();
        skip = opts["skip"].as<bool>();
        timeout = opts["timeout"].as<unsigned>();
        
        const auto &vals = item["values"].as<YAML::Node>();
        for (auto it = vals.begin(); it != vals.end(); ++it) {
            base[it->first.as<std::string>()] = it->second.as<double>();
        }
    }
    using target_parameter_vector_t = std::vector<functest::traits::target_parameter_t>;
    target_parameter_vector_t
    update_target_parameters(const target_parameter_vector_t &given) {
        if (given.size() != 1)
            return given;
        const auto &sp = given[0];
        auto section = sp.first;
        auto parameter = sp.second;
        if (section != "ALL" && parameter != "ALL")
            return given;
        target_parameter_vector_t result;
        for (const auto &i : base) {
            auto v_sp = helpers::str_split(i.first, '/');
            auto v_section = v_sp[0];
            auto v_parameter = v_sp[1];
            if (section != "ALL" && v_section != section)
                continue;
            if (parameter != "ALL" && v_parameter != parameter)
                continue;
            result.push_back(functest::traits::target_parameter_t { v_section, v_parameter });
        }
        return result;
    }
};

struct input_maker : public input_maker_base {
    std::string load_key = "-load";
    std::string result_key = "-result";
    std::string conf_key = "";
    std::string timeout_key = "-timeout";
    test_item_t testitem;
    test_scope<traits> &scope;
    input_maker(test_scope<traits> &_scope);
    virtual void make(std::string &input_yaml, std::string &psubmit_options, std::string &args);
};

struct output_maker : public output_maker_base {
    test_item_t testitem;
    test_scope<traits> scope;
    YAML::Emitter out;
    std::string outfile;
    output_maker(test_scope<traits> &_scope, const std::string &_outfile);
    ~output_maker();
    virtual void make(std::vector<std::shared_ptr<process>> &attempts);
};

} // namespace functest
