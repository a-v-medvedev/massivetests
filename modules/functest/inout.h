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

#if 0
struct common_params_t {
    std::string name;
    std::map<std::string, double> base;
    bool skip = false;
    unsigned timeout = 15;  // FIXME make it a cmdline param
    double tolerance = 1e-10; // FIXME make it a cmdline param
    void load(const std::string &_name) {
		
/*
        name = _name;
        std::ifstream in;
        in.open("test_items.yaml");
        auto stream = YAML::Load(in);
        if (!stream[name]) {
            skip = true;
            return;
        }
        const auto &item = stream[name].as<YAML::Node>();
        if (item["options"]) {
            const auto &opts = item["options"].as<YAML::Node>();
            if (opts["skip"]) {
                skip = opts["skip"].as<bool>();
            }
            if (opts["timeout"]) {
                timeout = opts["timeout"].as<unsigned>();
            }
            if (opts["tolerance"]) {
                tolerance = opts["tolerance"].as<double>();
            } else {
                
            }
        }
        const auto &vals = item["values"].as<YAML::Node>();
        for (auto it = vals.begin(); it != vals.end(); ++it) {
            base[it->first.as<std::string>()] = it->second.as<double>();
        }
*/
    }
};
#endif

struct test_item_t {
    std::string name;
    std::map<std::string, double> base;
	std::map<std::string, std::map<std::pair<int, int>, double>> tolerance_variations;
    bool skip = false;
    unsigned timeout = 15;  // FIXME make it a cmdline param
    double tolerance = 1e-10; // FIXME make it a cmdline param
	void load_tolerance_from_common_params(YAML::Node &stream, const std::string &param) {
		std::cout << ">> Loading tolerance dict for: " << param << std::endl;
		if (!stream["common_params"]) 
			return;
		if (!(stream["common_params"]).as<YAML::Node>()["tolerance"]) 
			return;
        const auto &cp = stream["common_params"].as<YAML::Node>();
		const auto &dict = cp["tolerance"].as<YAML::Node>();
		if (!dict[param])
			return;
		double default_tolerance = tolerance;
		const auto &tolerance_dict = dict[param].as<YAML::Node>();
		if (tolerance_dict["default"]) {
			default_tolerance = (tolerance_dict["default"]).as<double>();
		}
		std::pair<int, int> zero{0, 0};
		tolerance_variations[param][zero] = default_tolerance;
		for (YAML::const_iterator it = tolerance_dict.begin(); it != tolerance_dict.end(); ++it) {
			if (it->first.as<std::string>() == "default")
				continue;
			auto s = helpers::str_split(it->first.as<std::string>(), '/');
			if (s.size() != 2)
				continue;
			bool success = true;
			int n, ppn;
			try {
				n = std::stoi(s[0]);
				ppn = std::stoi(s[1]);
			}
			catch(...) {
				success = false;
			}
			if (!success)
				continue;
			std::pair<int, int> id(n, ppn);
			tolerance_variations[param][id] = (it->second).as<double>();
			std::cout << ">> saving tolerance value: " << (it->second).as<double>() << " for {" << n << "," << ppn << "}" << std::endl;
		}
	}
	double get_tolerance(const std::string &param, int n, int ppn) {
		double default_tolerance = tolerance;
		std::pair<int, int> zero(0, 0);
		std::pair<int, int> id(n, ppn);
		if (tolerance_variations.find(param) != tolerance_variations.end()) {
            auto &tvp = tolerance_variations[param];
			if (tvp.find(zero) != tvp.end()) {
				default_tolerance = tvp[zero];
			}
			if (tvp.find(id) != tvp.end()) {
				return tvp[id];
			}
		}
		return default_tolerance;
	}
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
        if (item["options"]) {
            const auto &opts = item["options"].as<YAML::Node>();
            if (opts["skip"]) {
                skip = opts["skip"].as<bool>();
            }
            if (opts["timeout"]) {
                timeout = opts["timeout"].as<unsigned>();
            }
            if (opts["tolerance"]) {
                tolerance = opts["tolerance"].as<double>();
            } else {
                
            }
        }
        const auto &vals = item["values"].as<YAML::Node>();
        for (auto it = vals.begin(); it != vals.end(); ++it) {
			const auto &param = it->first.as<std::string>();
            base[param] = it->second.as<double>();
			load_tolerance_from_common_params(stream, param);
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
    test_scope<traits> &scope;
    YAML::Emitter out;
    std::string outfile;
    output_maker(test_scope<traits> &_scope, const std::string &_outfile);
    ~output_maker();
    virtual void make(std::vector<std::shared_ptr<process>> &attempts);
};

} // namespace functest
