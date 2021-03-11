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

template <typename TRAITS>
struct test_scope {
    using workload_conf_t = typename TRAITS::workload_conf_t;
    using parallel_conf_t = typename TRAITS::parallel_conf_t;
    using target_parameter_t = typename TRAITS::target_parameter_t;
    using workload_size_t = typename TRAITS::workload_size_t;
    int id = 0;
    static int counter;
    test_scope() { id = ++counter; }
    test_scope(const workload_conf_t &_workload_conf,
               const std::vector<parallel_conf_t> &_parallel_confs,
               const std::vector<target_parameter_t> &_target_parameters,
               const std::vector<workload_size_t> &_workload_sizes)
        : test_scope() {
        workload_conf = _workload_conf;
        parallel_confs = _parallel_confs;
        target_parameters = _target_parameters;
        workload_sizes = _workload_sizes;
    }
    workload_conf_t workload_conf;
    std::vector<parallel_conf_t> parallel_confs;
    std::vector<target_parameter_t> target_parameters;
    std::vector<workload_size_t> workload_sizes;
};
