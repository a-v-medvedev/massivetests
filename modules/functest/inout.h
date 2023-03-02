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

#include "modules/functest/traits.h"
#include "scope.h"
#include "process.h"

namespace functest {

struct test_item_t {
    std::string name;
    std::map<std::string, double> base;
    protected:
	std::vector<std::pair<std::string, std::string>> prereq;
	std::map<std::string, std::map<std::pair<int, int>, double>> tolerance_variations;
	std::map<std::pair<int, int>, unsigned> timeout_variations;
    std::map<std::string, std::map<std::pair<int, int>, bool>> skip_flag_variations;
    bool skip = false;
    unsigned timeout = 0;
    unsigned def_timeout = 15;  // FIXME make it a cmdline param
    double tolerance = -1;
    double def_tolerance = 1e-8; // FIXME make it a cmdline param
    public:
    std::string preproc, postproc;
    bool get_int_pair_from_string(const std::string &str, std::pair<int, int> &output) {
        auto s = helpers::str_split(str, '/');
        if (s.size() != 2)
            return false;
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
            return false;
        output.first = n;
        output.second = ppn;
        return true;
    }
	void load_simple_common_params(YAML::Node &stream) {
		if (!stream["common_params"]) 
			return;
		bool have_timeout_key = stream["common_params"].as<YAML::Node>()["timeout"]; 
        const auto &cp = stream["common_params"].as<YAML::Node>();
        if (have_timeout_key)
		{
			const auto &timeout_dict = cp["timeout"].as<YAML::Node>();
			std::pair<int, int> zero{0, 0};
			unsigned default_timeout = timeout;
			if (timeout_dict["default"]) {
				default_timeout = timeout_dict["default"].as<unsigned>();
			}
			timeout_variations[zero] = default_timeout;
			for (YAML::const_iterator it = timeout_dict.begin(); it != timeout_dict.end(); ++it) {
				const auto& key = it->first.as<std::string>();
				unsigned val = it->second.as<unsigned>();
				if (key == "default")
					continue;
				std::pair<int, int> id;
				if (!get_int_pair_from_string(key, id))
					continue;
				timeout_variations[id] = val;
			}
		}
	}

	void load_tolerance_from_common_params(YAML::Node &stream, const std::string &param) {
		if (!stream["common_params"]) 
			return;
		if (!(stream["common_params"]).as<YAML::Node>()["tolerance"]) 
			return;
        const auto &cp = stream["common_params"].as<YAML::Node>();
		const auto &dict = cp["tolerance"].as<YAML::Node>();
		if (!dict[param])
			return;
		const auto &tolerance_dict = dict[param].as<YAML::Node>();
        double default_tolerance = tolerance;
        if (tolerance_dict["default"]) {
            default_tolerance = tolerance_dict["default"].as<double>();
        }
        std::pair<int, int> zero{0, 0};
        tolerance_variations[param][zero] = default_tolerance;
        for (YAML::const_iterator it = tolerance_dict.begin(); it != tolerance_dict.end(); ++it) {
            const auto& key = it->first.as<std::string>();
            bool val = it->second.as<double>();
            if (key == "default")
                continue;
            std::pair<int, int> id;
            if (!get_int_pair_from_string(key, id))
                continue;            
            tolerance_variations[param][id] = val;
        }
	}

    void load_skip_flag_from_common_params(YAML::Node &stream, const std::string &param) {
		if (!stream["common_params"]) 
			return;
		if (!(stream["common_params"]).as<YAML::Node>()["skip"]) 
			return;
        const auto &cp = stream["common_params"].as<YAML::Node>();
		const auto &dict = cp["skip"].as<YAML::Node>();
		if (!dict[param])
			return;
		const auto &skip_dict = dict[param].as<YAML::Node>();
        bool default_skip = false;
        if (skip_dict["default"]) {
            default_skip = skip_dict["default"].as<bool>();
        }
        std::pair<int, int> zero{0, 0};
        skip_flag_variations[param][zero] = default_skip;
        for (YAML::const_iterator it = skip_dict.begin(); it != skip_dict.end(); ++it) {
            const auto& key = it->first.as<std::string>();
            bool val = it->second.as<bool>();
            if (key == "default")
                continue;
            std::pair<int, int> id;
            if (!get_int_pair_from_string(key, id))
                continue;            
            skip_flag_variations[param][id] = val;
        }
	}

	double get_tolerance(const std::string &param, int n, int ppn = 1) {
        if (tolerance != -1) {
            return tolerance;
        }
		double default_tolerance = def_tolerance;
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

    unsigned get_timeout(int n, int ppn = 1) const {
        if (timeout != 0) {
            return timeout;
        }
        unsigned default_timeout = def_timeout;
        std::pair<int, int> zero(0, 0);
        std::pair<int, int> id(n, ppn);
		if (timeout_variations.find(zero) != timeout_variations.end()) {
			default_timeout = timeout_variations.find(zero)->second;
		}
		if (timeout_variations.find(id) != timeout_variations.end()) {
			return timeout_variations.find(id)->second;
		}
		return default_timeout;
    }

	bool get_prerequisites(std::vector<std::pair<std::string, std::string>> &_prereq) {
		_prereq  = prereq;
	    return prereq.size() != 0;	
	}

    unsigned get_skip_flag(const std::string &param, int n, int ppn = 1) {
        unsigned default_skip_flag = skip;
        std::pair<int, int> zero(0, 0);
        std::pair<int, int> id(n, ppn);
        if (skip_flag_variations.find(param) != skip_flag_variations.end()) {
            auto &skvp = skip_flag_variations[param];
            if (skvp.find(zero) != skvp.end()) {
                default_skip_flag = skvp[zero];
            }
            if (skvp.find(id) != skvp.end()) {
                return skvp[id];
            }
        }
		return default_skip_flag;
    }

    void load(const std::string &_name) {
        name = _name;
        auto s = helpers::str_split(name, '/');
        std::string specialized = std::string("test_items_") + s[0] + std::string(".yaml");
        std::string generic = "test_items.yaml";
        std::ifstream in;
        in.open(specialized);
        if (!in.good()) {
            in.open(generic);
            if (!in.good()) {
                assert(0 && "can't find a parameters file to read from.");
            }
        }
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
            }
            if (opts["preproc"]) {
                preproc = opts["preproc"].as<std::string>();
            }
            if (opts["postproc"]) {
                postproc = opts["postproc"].as<std::string>();
            }

        }
        if (item["prerequisites"]) {
            const auto &pnode = item["prerequisites"].as<YAML::Node>();
            for (auto it = pnode.begin(); it != pnode.end(); ++it) {
                const auto &from = it->first.as<std::string>();
                const auto &to = it->second.as<std::string>();
                prereq.push_back(std::pair<std::string, std::string>(from, to));
            }
        }    

        const auto &vals = item["values"].as<YAML::Node>();
        for (auto it = vals.begin(); it != vals.end(); ++it) {
			const auto &param = it->first.as<std::string>();
            base[param] = it->second.as<double>();
			load_tolerance_from_common_params(stream, param);
        }
        load_skip_flag_from_common_params(stream, name);
		load_simple_common_params(stream);
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

template <typename parallel_conf_t>
struct input_maker : public input_maker_base<parallel_conf_t> {
    std::string load_key = "-load";
    std::string result_key = "-result";
    std::string conf_key = "";
    std::string timeout_key = "-timeout";
    test_item_t testitem;
    test_scope<traits> &scope;
    input_maker(test_scope<traits> &_scope);
    virtual bool make(const parallel_conf_t &pconf, execution_environment &env);
	void do_substs(const parallel_conf_t &pconf, std::string &filename);
	bool file_exists(const parallel_conf_t &pconf, const std::string &filename_);
	bool exec_shell_command(const parallel_conf_t &pconf, const test_item_t &testitem,
                            const std::string &script, std::string &result, int &status);
};

template <typename parallel_conf_t>
struct output_maker : public output_maker_base<parallel_conf_t> {
    test_item_t testitem;
    test_scope<traits> &scope;
    YAML::Emitter out;
    std::string outfile;
    output_maker(test_scope<traits> &_scope, const std::string &_outfile);
    ~output_maker();
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts);
};

} // namespace functest
