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
#include <utility>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include "argsparser.h"

namespace helpers {

static inline bool file_exists(const std::string &file) {
    return access(file.c_str(), F_OK) == 0;
}

static inline bool file_is_exec(const std::string &file) {
    return access(file.c_str(), X_OK) == 0;
}

static inline void subst(std::string &str, const std::string &pattern, const std::string &substitute) {
    str = std::regex_replace(str, std::regex(pattern), substitute);
}

static inline void subst(std::string &str, char pattern, char substitute) {
    size_t pos = 0;
    std::string str_substitute(1, substitute);
    while ((pos = str.find(pattern, pos)) != std::string::npos) {
        str.replace(pos, 1, str_substitute);
        pos += 1; // skip the replaced character
    }
}

static inline std::string which(const std::string& command) {
    static std::vector<std::string> dirs;
    if (dirs.empty()) {
        const char *path = std::getenv("PATH");
        if (path == nullptr) {
            dirs.push_back(".");
        } else {
            const char *start = path;
            while (*start != '\0') {
                const char *end = start;
                while (*end != '\0' && *end != ':') {
                    end++;
                }
                if (end > start) {
                    dirs.emplace_back(start, end - start);
                }
                if (*end == ':') {
                    end++;
                }
                start = end;
            }
        }
    }
    for (const auto& dir : dirs) {
        std::string fullpath = dir + "/" + command;
        struct stat st;
        if (stat(fullpath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
            return fullpath;
        }
    }
    return "";
}

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

static inline bool is_int(const std::string& s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+')))
        return false;
    char* p;
    strtol(s.c_str(), &p, 10);
    return (*p == 0);
}

static inline bool is_float(const std::string& s) {
    std::istringstream iss(s);
    double value;
    return (iss >> value) && (iss.eof());
}

static inline bool is_bool(const std::string& s) {
    return s == "true" || s == "false";
}

template <typename T>
T str2value(const std::string &value);

template <>
std::string str2value<std::string>(const std::string &value);

template <>
int str2value<int>(const std::string &value);

template <>
size_t str2value<size_t>(const std::string &value);

template <>
double str2value<double>(const std::string &value);

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

template <typename T>
std::string value2str(const T &x);

template <>
std::string value2str<double>(const double &x);

template <>
std::string value2str<int>(const int &x);

template <>
std::string value2str<bool>(const bool &x);

template <>
std::string value2str<std::string>(const std::string &x);

static inline bool contains(const std::string &str, char ch) {
    return str.find(ch) != std::string::npos;
}

/*
template <typename KEY, typename VALUE>
std::vector<std::pair<KEY, VALUE>> parsers_map_to_vector(const args_parser &parser,
                                                         const std::string &arg_name) {
    using element_t = std::pair<KEY, VALUE>; 
    std::vector<element_t> result;
    std::map<std::string, std::string> result_map;
    parser.get(arg_name, result_map);
    for (const auto &elem : result_map) {
        result.push_back(element_t(str2value<KEY>(elem.first), str2value<VALUE>(elem.second)));
    }
    std::sort(result.begin(), result.end(), [](const element_t &val1, const element_t &val2){ return val1.first < val2.first; });
    return result;
}
*/

template <typename TUPLE>
std::vector<TUPLE> parsers_vector_to_tuple(const args_parser &parser, const std::string &arg_name) {
    constexpr auto ts = std::tuple_size<TUPLE>{};
    std::vector<TUPLE> result;
    std::vector<std::string> result_as_str;
    parser.get(arg_name, result_as_str);
    for (const auto &elem : result_as_str) {
        auto A = str_split(elem, ':');
        if (A.size() > ts) {
            throw std::runtime_error("parsers_vector_to_tuple: more data than the tuple size!");
        }
        TUPLE t;
        for (size_t i = 0; i < std::tuple_size<TUPLE>{}; i++) {
            switch (i) {
                case 0: if constexpr (0 < ts) { using elem_t = typename std::tuple_element<0, TUPLE>::type; std::get<0>(t) = str2value<elem_t>(A[i]); break; }
                case 1: if constexpr (1 < ts) { using elem_t = typename std::tuple_element<1, TUPLE>::type; std::get<1>(t) = str2value<elem_t>(A[i]); break; }
                case 2: if constexpr (2 < ts) { using elem_t = typename std::tuple_element<2, TUPLE>::type; std::get<2>(t) = str2value<elem_t>(A[i]); break; }
                case 3: if constexpr (3 < ts) { using elem_t = typename std::tuple_element<3, TUPLE>::type; std::get<3>(t) = str2value<elem_t>(A[i]); break; }
                case 4: if constexpr (4 < ts) { using elem_t = typename std::tuple_element<4, TUPLE>::type; std::get<4>(t) = str2value<elem_t>(A[i]); break; }
                case 5: if constexpr (5 < ts) { using elem_t = typename std::tuple_element<5, TUPLE>::type; std::get<5>(t) = str2value<elem_t>(A[i]); break; }
                case 6: if constexpr (6 < ts) { using elem_t = typename std::tuple_element<6, TUPLE>::type; std::get<6>(t) = str2value<elem_t>(A[i]); break; }
                case 7: if constexpr (7 < ts) { using elem_t = typename std::tuple_element<7, TUPLE>::type; std::get<7>(t) = str2value<elem_t>(A[i]); break; }
                case 8: if constexpr (8 < ts) { using elem_t = typename std::tuple_element<8, TUPLE>::type; std::get<8>(t) = str2value<elem_t>(A[i]); break; }
                case 9: if constexpr (9 < ts) { using elem_t = typename std::tuple_element<9, TUPLE>::type; std::get<9>(t) = str2value<elem_t>(A[i]); break; }
                default: throw std::runtime_error("parsers_vector_to_tuple: can't handle this tuple.");
            }
        }
        result.push_back(t);
    }
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
        result.push_back(std::pair<KEY, VALUE>(str2value<KEY>(A[0]), str2value<VALUE>(A[1])));
    }
    return result;
}

static inline void trunc_file(const std::string &name) {
    std::ofstream ofs(name, std::ios_base::out | std::ios_base::trunc);
}

static inline bool try_to_open_file(std::ifstream &in, const std::string &name, unsigned nattempts, unsigned sleeptime) {
    bool success = false;
    unsigned n = 0;
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

template <class CONTAINER>
void yaml_out_map(YAML::Emitter &out, const std::string &key, const CONTAINER &vals) {
    out << YAML::Key << key.c_str() << YAML::Value << YAML::Flow;
    out << YAML::Flow << YAML::BeginMap;
    for (auto &v : vals) {
        out << YAML::Key << v.first << YAML::Value << v.second << YAML::Flow;
    }
    out << YAML::EndMap;
}

static inline YAML::Node advance_yaml_node(const YAML::Node &stream, const std::string &section) {
    YAML::Node node = stream;
    auto sarr = helpers::str_split(section, '/');
    for (const auto &subsection : sarr) {
        if (subsection.empty())
            break;
        if (!node.IsMap()) {
            return YAML::Node(YAML::NodeType::Undefined);
        }
        if (!node[subsection]) {
            return YAML::Node(YAML::NodeType::Undefined);
        }
        node.reset(node[subsection].as<YAML::Node>());
    }
    return node;
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

#define YAML_OUT_MAP(KEY, VALS) \
    { \
        using CONTAINER = decltype(VALS); \
        helpers::yaml_out_map<CONTAINER>(out, KEY, VALS); \
    }

} // namespace helpers
