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
#include <sstream>
#include <variant>

#include "modules/functest/traits.h"
#include "scope.h"
#include "process.h"
#include "comparator.h"

namespace functest {

using descr_t = std::variant<double, int, bool, std::string, std::vector<std::string>>;
enum tolerance_float_kind_t { ABSOLUTE, RELATIVE };

struct test_item_t {
    std::string name;
    std::map<std::string, descr_t> base;
    protected:
	std::vector<std::pair<std::string, std::string>> prereq;
    std::map<std::pair<int, int>, bool> skip_variations;
	std::map<std::pair<int, int>, unsigned> timeout_variations;
	std::map<std::pair<int, int>, double> tolerance_float_variations;
	std::map<std::pair<int, int>, unsigned> tolerance_int_variations;
    bool skip;
    unsigned timeout;
    double tolerance_float;
    unsigned tolerance_int;
    tolerance_float_kind_t tolerance_float_kind;
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

    template <typename T>
    void load_variations(const YAML::Node &dict, T default_value, std::map<std::pair<int, int>, T> &result) {
        static const std::pair<int, int> zero{0, 0};
        result[zero] = default_value;
        for (YAML::const_iterator it = dict.begin(); it != dict.end(); ++it) {
            const auto& key = it->first.as<std::string>();
            unsigned value = it->second.as<T>();
            std::pair<int, int> id;
            if (!get_int_pair_from_string(key, id))
                continue;
            result[id] = value;
        }
    }

    template <typename T>
    T search_variations_by_n_ppn(int n, int ppn, const std::map<std::pair<int, int>, T> &variations, T default_value) const {
        std::pair<int, int> zero(0, 0);
        std::pair<int, int> id(n, ppn);
        if (variations.find(zero) != variations.end()) {
            default_value = variations.find(zero)->second;
        }
        if (variations.find(id) != variations.end()) {
            return variations.find(id)->second;
        }
		return default_value;
    }

    public:
	void load_basic_common_params(const YAML::Node &stream) {
        if (stream["timeout"]) {
            timeout = stream["timeout"].as<unsigned>();
        }
        if (stream["skip"]) {
            skip = stream["skip"].as<bool>();
        }
        if (stream["tolerance_int"]) {
            tolerance_int = stream["tolerance_int"].as<unsigned>();
        }
        if (stream["tolerance_float"]) {
            tolerance_float = stream["tolerance_float"].as<double>();
        }
        if (stream["tolerance_float_kind"]) {
            auto kind = stream["tolerance_float_kind"].as<std::string>();
            if (kind != "absolute" && kind != "relative") {
                assert(0 && "wrong or not implemented tolerance kind");
            }
            tolerance_float_kind = (kind == "absolute" ? ABSOLUTE : RELATIVE);
        }
        if (stream["per-pconf"]) {
            const auto &ppconf = stream["per-pconf"].as<YAML::Node>();
            if (!ppconf.IsMap()) 
                return;
            if (ppconf["timeout"]) {
                load_variations<unsigned>(ppconf["timeout"].as<YAML::Node>(), timeout, timeout_variations);           
            }
            if (ppconf["skip"]) {
                load_variations<bool>(ppconf["skip"].as<YAML::Node>(), skip, skip_variations);
            }
            if (ppconf["tolerance_int"]) {
                load_variations<unsigned>(ppconf["tolerance_int"].as<YAML::Node>(), tolerance_int, tolerance_int_variations);
            }
            if (ppconf["tolerance_float"]) {
                load_variations<double>(ppconf["tolerance_float"].as<YAML::Node>(), tolerance_float, tolerance_float_variations);
            }
        }
    }

    bool get_prerequisites_flag() const {
	    return prereq.size() != 0;	
	}

	const std::vector<std::pair<std::string, std::string>> &get_prerequisites() const {
		return prereq;
	}

    std::string get_preproc() const {
        return preproc;
    }

    std::string get_postproc() const {
        return postproc;
    }

    unsigned get_skip_flag(int n, int ppn = 1) const {
        return search_variations_by_n_ppn<bool>(n, ppn, skip_variations, skip);    
    }

    unsigned get_tolerance_int(int n, int ppn = 1) const {
        return search_variations_by_n_ppn<unsigned>(n, ppn, tolerance_int_variations, tolerance_int);    
    }

    double get_tolerance_float(int n, int ppn = 1) const {
        return search_variations_by_n_ppn<double>(n, ppn, tolerance_float_variations, tolerance_float);    
    }

    tolerance_float_kind_t get_tolerance_float_kind() const {
        return tolerance_float_kind;
    }

    unsigned get_timeout(int n, int ppn = 1) const {
        return search_variations_by_n_ppn<unsigned>(n, ppn, timeout_variations, timeout);    
    }

    std::shared_ptr<comparator_t> get_comparator(const std::string &parameter_code, 
                                                 int n, 
                                                 int ppn, 
                                                 const std::string &indir) {
        bool is_scalar = std::holds_alternative<double>(base[parameter_code]) ||
                         std::holds_alternative<int>(base[parameter_code]) ||
                         std::holds_alternative<bool>(base[parameter_code]);
        if (is_scalar) {
            std::shared_ptr<comparator_t> retvalue;
            if (std::holds_alternative<double>(base[parameter_code])) {
                if (get_tolerance_float_kind() == ABSOLUTE) {
                    auto c = std::make_shared<absolute_numeric_value_comparator<double>>();
                    c->base = std::get<double>(base[parameter_code]);
                    c->tolerance = get_tolerance_float(n, ppn);
                    retvalue = c;
                } else if (get_tolerance_float_kind() == RELATIVE) {
                    auto c = std::make_shared<relative_numeric_value_comparator<double>>();
                    c->base = std::get<double>(base[parameter_code]);
                    c->tolerance = get_tolerance_float(n, ppn);
                    retvalue = c;
                } else {
                    assert(0 && "not implemented");
                }
            } else if (std::holds_alternative<int>(base[parameter_code])) {
                auto c = std::make_shared<absolute_numeric_value_comparator<int>>();
                c->base = std::get<int>(base[parameter_code]);
                c->tolerance = get_tolerance_int(n, ppn);
                retvalue = c;
            } else if (std::holds_alternative<bool>(base[parameter_code])) {
                auto c = std::make_shared<absolute_nonnumeric_value_comparator<bool>>();
                c->base = std::get<bool>(base[parameter_code]);
                retvalue = c;
            } else if (std::holds_alternative<std::string>(base[parameter_code])) {
                auto c = std::make_shared<absolute_nonnumeric_value_comparator<std::string>>();
                c->base = std::get<std::string>(base[parameter_code]);
                retvalue = c;
            }
            retvalue->parameter_code = parameter_code;
            retvalue->dir = indir;
            return retvalue;
        } else {
            if (std::holds_alternative<std::vector<std::string>>(base[parameter_code])) {
                std::shared_ptr<comparator_t> retvalue;
                auto &tokens = std::get<std::vector<std::string>>(base[parameter_code]);
                assert(tokens.size() != 0);
                auto &keyword = tokens[0];
                if (keyword == "relative") {
                    auto c = std::make_shared<relative_numeric_value_comparator<double>>();
                    assert(tokens.size() == 3);
                    c->base = helpers::str2value<double>(tokens[1]);
                    if (helpers::contains(tokens[2], '%')) {
                        auto tks = helpers::str_split(tokens[2], '%');
                        assert(tks.size() == 1);
                        c->tolerance = helpers::str2value<double>(tks[0]) * 0.01;
                    } else {
                        c->tolerance =  helpers::str2value<double>(tokens[2]);
                    }
                    retvalue = c;
                }
                if (keyword == "absolute") {
                    assert(tokens.size() == 3);
                    if (helpers::is_int(tokens[1]) && helpers::is_int(tokens[2])) {
                        auto c = std::make_shared<absolute_numeric_value_comparator<int>>();
                        c->base = helpers::str2value<int>(tokens[1]);
                        c->tolerance = helpers::str2value<int>(tokens[2]);
                        retvalue = c;
                    } else {
                        auto c = std::make_shared<absolute_numeric_value_comparator<double>>();
                        c->base = helpers::str2value<double>(tokens[1]);
                        c->tolerance = helpers::str2value<double>(tokens[2]);
                        retvalue = c;
                    }
                }
                if (keyword == "equal") {
                    assert(tokens.size() == 2);
                    auto value = tokens[1];
                    if (helpers::is_int(value)) {
                        auto c = std::make_shared<absolute_numeric_value_comparator<int>>();
                        c->base = helpers::str2value<int>(value);
                        c->tolerance = 0;
                        retvalue = c;
                    } else if (helpers::is_float(value)) {
                        assert(0 && "equality comparator is not available for floating point constants.");
                    } else if (helpers::is_bool(value)) {
                        auto c = std::make_shared<absolute_nonnumeric_value_comparator<bool>>();
                        c->base = helpers::str2value<bool>(value);
                        retvalue = c;
                    } else {
                        auto c = std::make_shared<absolute_nonnumeric_value_comparator<std::string>>();
                        c->base = value;
                        retvalue = c;
                    }
                }
                if (keyword == "oneof") {
                    assert(tokens.size() > 1);
                    bool allint = true;
                    std::vector<int> ivec;
                    for (size_t i = 1; i < tokens.size(); ++i) {
                        if (!helpers::is_int(tokens[i])) {
                            allint = false;
                            break;
                        } else {
                            ivec.push_back(helpers::str2value<int>(tokens[i]));
                        }
                    }
                    if (allint) {
                        auto c = std::make_shared<oneof_value_comparator<int>>();
                        c->base = ivec;
                        retvalue = c;
                    } else {
                        auto c = std::make_shared<oneof_value_comparator<std::string>>();
                        c->base.resize(tokens.size() - 1);
                        for (size_t i = 1; i < tokens.size(); ++i) {
                            c->base[i - 1] = tokens[i];
                        }
                        retvalue = c;
                    }
                }
                if (keyword == "auxvalue") {
                    assert(tokens.size() == 4);
                    auto c = std::make_shared<auxvalue_collector>();
                    c->type = tokens[1];
                    c->averaging = tokens[2];
                    c->name = tokens[3];
                    retvalue = c;
                }
                retvalue->parameter_code = parameter_code;
                retvalue->dir = indir;
                return retvalue;
            }
        }
        return std::shared_ptr<comparator_t>(nullptr);
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
        timeout = functest::traits::default_timeout;
        skip = false;
        tolerance_int = functest::traits::default_tolerance_int;
        tolerance_float = functest::traits::default_tolerance_float;
        tolerance_float_kind = ABSOLUTE;
        if (stream["common_params"]) {
            const auto &cp = stream["common_params"];
		    load_basic_common_params(cp);
            //... some additional opts from common_params??
        }

        const auto &item = stream[name].as<YAML::Node>();
        if (item["options"]) {
            const auto &opts = item["options"].as<YAML::Node>();
            load_basic_common_params(opts);
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
            auto &descr = it->second;
            if (descr.IsSequence()) {
                base[param] = std::vector<std::string>();
                auto &tokens = std::get<std::vector<std::string>>(base[param]);
                for (YAML::const_iterator it = descr.begin(); it != descr.end(); ++it) {
                    tokens.push_back(it->as<std::string>());
                }
            } else if (descr.IsScalar()) {
                std::string str = descr.as<std::string>();
                bool converted = false;
                if (!converted && helpers::is_int(str)) {
                    base[param] = descr.as<int>();
                    converted = true;
                }
                if (!converted && helpers::is_float(str)) {
                    base[param] = descr.as<double>();
                    converted = true;
                }
                if (!converted && helpers::is_bool(str)) {
                    base[param] = descr.as<bool>();
                    converted = true;
                }
                if (!converted) {
                    base[param] = descr.as<std::string>();
                }
            }
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

} // namespace functest
