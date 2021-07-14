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
#include <utility>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unistd.h>
#include <assert.h>
#include "argsparser.h"

namespace helpers {

static inline std::vector<std::string> str_split(const std::string &s, char delimiter) {
    std::vector<std::string> result;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

static inline std::vector<int> vstr_to_vint(const std::vector<std::string> &from) {
    std::vector<int> to;
    to.clear();
    for (auto &s : from) {
        int x = std::stoi(s);
        to.push_back(x);
    }
    return to;
}

template <typename T>
T convert(const std::string &value);

template <>
std::string convert<std::string>(const std::string &value);

template <>
int convert<int>(const std::string &value);

template <>
size_t convert<size_t>(const std::string &value);

static inline std::string bool2str(bool v) {
    return v ? "true" : "false";
}

static inline std::string flt2str(double x) {
    std::ostringstream ss;
    ss.setf(std::ios::scientific, std::ios::floatfield);
    ss.precision(12);
    ss << x;
    return ss.str();
}


template <typename KEY, typename VALUE>
std::vector<std::pair<KEY, VALUE>> parsers_map_to_vector(const args_parser &parser,
                                                         const std::string &arg_name) {
    using element_t = std::pair<KEY, VALUE>; 
    std::vector<element_t> result;
    std::map<std::string, std::string> result_map;
    parser.get(arg_name, result_map);
    for (const auto &elem : result_map) {
        result.push_back(element_t(convert<KEY>(elem.first), convert<VALUE>(elem.second)));
    }
    std::sort(result.begin(), result.end(), [](const element_t &val1, const element_t &val2){ return val1.first < val2.first; });
    return result;
}

template <typename KEY, typename VALUE>
std::vector<std::pair<KEY, VALUE>> parsers_vector_to_vector(const args_parser &parser,
                                                            const std::string &arg_name) {
    std::vector<std::pair<KEY, VALUE>> result;
    std::vector<std::string> result_as_str;
    parser.get(arg_name, result_as_str);
    for (const auto &elem : result_as_str) {
        auto A = str_split(elem, ':');
        if (A.size() != 2) {
            throw 0;
        }
        result.push_back(std::pair<KEY, VALUE>(convert<KEY>(A[0]), convert<VALUE>(A[1])));
    }
    return result;
}

static inline void trunc_file(const std::string &name) {
    std::ofstream ofs(name, std::ios_base::out | std::ios_base::trunc);
}

template <int nattempts, int sleeptime>
static inline bool try_to_open_file(std::ifstream &in, const std::string &name) {
    bool success = false;
    int n = 0;
    while (!success) {
        in.open(name);
        if (!in.is_open()) {
            if (n++ > nattempts)
                return false;
            ;
            usleep(sleeptime * 1000);
            continue;
        }
        break;
    }
    return true;
}

static inline double average(const std::vector<double> &vals) {
    assert(vals.size() != 0);
    return vals[vals.size() / 2];
}

template <typename T>
std::string conf_to_string(const T &conf) {
    std::stringstream ss;
    ss << conf;
    return ss.str();
}

//#define YAML_IN(KEY, VALUE, TYPE) if (node[KEY]) { VALUE = node[KEY].as<YAML::Node>().as<TYPE>();
//}
#define YAML_OUT(KEY, VALUE) out << YAML::Key << YAML::Flow << KEY << YAML::Value << VALUE;

template <class CONTAINER, typename ELEM_TYPE, class FUNC>
static inline void yaml_out_seq(YAML::Emitter &out, const std::string &key, const CONTAINER &vals,
                                FUNC getval, int nesting = 1) {
    out << YAML::Key << key.c_str() << YAML::Value << YAML::Flow;
    for (int i = 0; i < nesting; i++) {
        out << (i ? YAML::Newline : YAML::Flow) << YAML::BeginSeq;
    }
    for (auto &it : vals) {
        out << getval(it) << YAML::Flow;
    }
    for (int i = 0; i < nesting; i++) {
        (i ? out << YAML::Newline << YAML::EndSeq : out << YAML::EndSeq);
    }
}

#define YAML_OUT_SEQ(KEY, VALS, FUNCBODY)                                                          \
    {                                                                                              \
        using CONTAINER = decltype(VALS);                                                          \
        using ELEM_TYPE = typename CONTAINER::value_type;                                                   \
        const auto &fn = [](ELEM_TYPE v) FUNCBODY;                                                 \
        helpers::yaml_out_seq<CONTAINER, ELEM_TYPE>(out, KEY, VALS, fn);                           \
    }

#define YAML_OUT_SEQ_NESTED(KEY, VALS, FUNCBODY)                                                   \
    {                                                                                              \
        using CONTAINER = decltype(VALS);                                                          \
        using ELEM_TYPE = typename CONTAINER::value_type;                                                   \
        const auto &fn = [](ELEM_TYPE v) FUNCBODY;                                                 \
        helpers::yaml_out_seq<CONTAINER, ELEM_TYPE>(out, KEY, VALS, fn, 2);                        \
    }

} // namespace helpers
