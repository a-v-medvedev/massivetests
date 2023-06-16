/*
    This file is part of massivetest.

    Massivetest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    massivetest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with massivetest.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <memory>
#include <assert.h>
#include <cmath>

#include <yaml-cpp/yaml.h>

#include "modules/functest/traits.h"
#include "modules/functest/inout.h"

#include "helpers.h"
#include "results.h"

namespace functest {

template <typename val_t>
bool basic_value_comparator<val_t>::acquire_result_data_piece(const YAML::Node &stream, 
                                                              const std::string &section, 
                                                              const std::string &parameter) {
    const auto &sec = stream[section].as<YAML::Node>();
    if (parameter.find("[") != std::string::npos && 
        parameter.find("]") != std::string::npos) {
        auto pv = helpers::str_split(parameter, '[');
        assert(pv.size() == 2);
        auto idxv = helpers::str_split(pv[1], ']');
        assert(idxv.size() == 1);
        auto p = pv[0];
        auto idx = idxv[0];
        size_t i = std::stol(idx);
        if (!sec[p])
            return false;
        const auto &pn = sec[p].as<YAML::Node>();
        size_t n = 0;
        for (YAML::const_iterator it = pn.begin(); it != pn.end(); ++it) {
            if (i == n++) {
                result = it->as<val_t>();
                break;
            }
        }
    } else {
        if (!sec[parameter])
            return false;
        const auto &p = sec[parameter].as<YAML::Node>();
        result = p.as<val_t>();
    }
    return true;
}

template <typename val_t>    
status_t basic_value_comparator<val_t>::check_attempts_equality(std::vector<std::shared_ptr<comparator_t>> &v, std::string &comment) {
    if (v.size() == 1) {
        return status_t::P;
    }
    std::sort(v.begin(), v.end(), [](auto a, auto b) -> bool { return *a < *b; });
    auto &front = *(reinterpret_cast<basic_value_comparator<val_t> *>(&(*v.front())));
    auto &back = *(reinterpret_cast<basic_value_comparator<val_t> *>(&(*v.back())));
    if (front.result != back.result) {
        if (functest::traits::debug) {
            std::cout << ">> functest: v.front() != v.back(). ATTEMPTS COMPARISON FAILED!" << std::endl;
        }
        comment = std::string("Attempts comparison failed section/parameter=") + parameter_code + 
            std::string(" front=") + helpers::value2str(front.result) +
            std::string(" back=") + helpers::value2str(back.result) +
            std::string(" dir=") + front.dir +
            std::string(" dir2=") + back.dir;
        return status_t::F;
    }
    return status_t::P;
}

template <typename val_t>
bool basic_value_comparator<val_t>::operator<(const comparator_t &other_) const {
    auto &other = *(reinterpret_cast<const basic_value_comparator<val_t> *>(&other_));
    return result < other.result;
}

template <typename val_t>
status_t absolute_numeric_value_comparator<val_t>::compare(std::string &comment) const {
    double diff = fabs(result - base); 
    if (diff > tolerance) {
        if (functest::traits::debug) {
            std::cout << ">> functest: diff > " << tolerance << ". GOLD VALUE COMPARISON FAILED!" << std::endl;
        }
        comment = std::string("Gold value comparison failed section/parameter=") + parameter_code + 
            std::string(" diff=") + helpers::flt2str(diff) + 
            std::string(" abs.tol=") + helpers::flt2str(tolerance) + 
            std::string(" expected=") + helpers::flt2str(base) + 
            std::string(" acquired=") + helpers::flt2str(result) +
            std::string(" dir=") + dir; 
        return status_t::F;
    }
    return status_t::P;
}

template <typename val_t>
status_t absolute_nonnumeric_value_comparator<val_t>::compare(std::string &comment) const {
    if (result != base) {
        if (functest::traits::debug) {
            std::cout << ">> functest: result!=base" << ". GOLD VALUE COMPARISON FAILED!" << std::endl;
        }
        comment = std::string("Gold value comparison failed section/parameter=") + parameter_code + 
            std::string(" expected=") + helpers::value2str(base) + 
            std::string(" acquired=") + helpers::value2str(result) +
            std::string(" dir=") + dir; 
        return status_t::F;
    }
    return status_t::P;
}

template class absolute_numeric_value_comparator<double>;
template class absolute_numeric_value_comparator<int>;
template class absolute_nonnumeric_value_comparator<bool>;
template class absolute_nonnumeric_value_comparator<std::string>;

template <typename val_t>
status_t relative_numeric_value_comparator<val_t>::compare(std::string &comment) const {
    double diff = fabs((result - base) / base); 
    if (diff > tolerance) {
        if (functest::traits::debug) {
            std::cout << ">> functest: diff/base > " << tolerance << ". GOLD VALUE COMPARISON FAILED!" << std::endl;
        }
        comment = std::string("Gold value comparison failed section/parameter=") + parameter_code + 
            std::string(" diff=") + helpers::flt2str(diff) + 
            std::string(" rel.tol=") + helpers::flt2str(tolerance) + 
            std::string(" expected=") + helpers::flt2str(base) + 
            std::string(" acquired=") + helpers::flt2str(result) +
            std::string(" dir=") + dir; 
        return status_t::F;
    }
    return status_t::P;
}

template class relative_numeric_value_comparator<double>;

}
