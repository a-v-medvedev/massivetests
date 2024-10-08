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

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include <unistd.h>
#include <assert.h>

#include "yaml-cpp/yaml.h"
#include "argsparser.h"

#include "inout_base.h"
#include "process.h"
#include "scope.h"

#include "module.h"
// FIXME hide it somewhere
template <>
int test_scope<MODULE::traits>::counter = 0;

#include "dispatcher.h"
#include "helpers.h"
#include "results.h"

template <typename TRAITS>
void start(const std::vector<std::shared_ptr<test_scope<TRAITS>>> &scopes,
           const std::string &outfile, int nqueued, int repeats) {
    TRAITS traits;
    dispatcher<TRAITS> disp(repeats, nqueued);
    for (auto &scopeptr : scopes) {
        test_scope<TRAITS> &scope = *(scopeptr.get());
        auto im = traits.make_input_maker(scope);
        auto om = traits.make_output_maker(scope, outfile);
        auto wc = scope.workload_conf;
        for (auto &pc : scope.parallel_confs) {
            for (int i = 0; i < repeats; i++)
                disp.enqueue(scope.id, wc, pc, im, om);
        }
    }
    while (!disp.check_if_all_finished()) {
        usleep(1000);
    }
}

template <typename TRAITS>
void parse_and_start(const args_parser &parser, int nqueued, int repeats) {
    TRAITS traits;
    auto workload_confs = traits.parse_and_make_target_parameters(parser, "workloads");
    auto target_parameters = traits.parse_and_make_target_parameters(parser, "parameters");
    auto parallel_confs = traits.parse_and_make_parallel_confs(parser, "scale");
    auto workparts = traits.parse_and_make_workparts(parser, "workparts");
    helpers::trunc_file("output_initial.yaml");
    start<TRAITS>(traits.make_scopes(workload_confs, parallel_confs, target_parameters, 
                  workparts),
                  "output_initial.yaml", nqueued, repeats);
}

int main(int argc, char **argv) {
    if (getenv("MASSIVE_TESTS_DEBUG") != nullptr) {
        MODULE::traits::debug = true;
    }
    args_parser parser(argc, argv);
    parser.add_vector<std::string>("workloads", "")
        .set_mode(args_parser::option::APPLY_DEFAULTS_ONLY_WHEN_MISSING);
    parser.add_vector<std::string>("parameters", "")
        .set_mode(args_parser::option::APPLY_DEFAULTS_ONLY_WHEN_MISSING);
    parser.add_vector<std::string>("scale", "")
        .set_mode(args_parser::option::APPLY_DEFAULTS_ONLY_WHEN_MISSING);
    parser.add_map("workparts", "", ',', ':');
    parser.add<int>("nqueued", 5);
    parser.add<int>("repeats", 10);
    std::string a = MODULESTR;
    if (!parser.parse())
        return 1;
    int nqueued = parser.get<int>("nqueued");
    int repeats = parser.get<int>("repeats");
    parse_and_start<MODULE::traits>(parser, nqueued, repeats);
    return 0;
}
