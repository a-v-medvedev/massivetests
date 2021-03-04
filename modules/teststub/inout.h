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

#include "modules/teststub/traits.h"
#include "scope.h"
#include "process.h"

namespace teststub {

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
};

struct input_maker : public input_maker_base {
    test_item_t testitem;
    bool was_written;
    test_scope<traits> &scope;
    input_maker(test_scope<traits> &_scope);
    void write_out(const std::string &input_file_name);
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

} // namespace teststub
