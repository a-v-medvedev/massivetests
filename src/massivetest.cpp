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
#include "modules/imb_async/traits.h"
#include "modules/imb_async/inout.h"
#include "dispatcher.h"
#include "helpers.h"
#include "results.h"

// FIXME hide it somewhere
template <>
int test_scope<imb_async::traits>::counter = 0;

template <typename TRAITS>
void start(const std::vector<std::shared_ptr<test_scope<TRAITS>>> &scopes,
           const std::string &outfile, int nqueued, int repeats) {
    TRAITS traits;
    dispatcher<TRAITS> disp(repeats, nqueued);
    for (auto &scopeptr : scopes) {
        test_scope<TRAITS> &scope = *(scopeptr.get());
        auto im = traits.make_input_maker(scope);
        auto om = traits.make_output_maker(scope, outfile);
        for (auto &nppn : scope.parallel_confs) {
            for (int i = 0; i < repeats; i++)
                disp.enqueue(nppn, im, om);
        }
    }
    while (!disp.check_if_all_finished()) {
        usleep(10000);
    }
}

template <typename TRAITS>
void parse_and_start(const args_parser &parser, int nqueued, int repeats) {
    TRAITS traits;
    auto target_parameters = traits.parse_and_make_target_parameters(parser, "workloads");
    auto parallel_confs = traits.parse_and_make_parallel_confs(parser, "scale");
    auto workload_sizes = traits.parse_and_make_workload_sizes(parser, "sizes");

    helpers::trunc_file("output_initial.yaml");
    start<TRAITS>({traits.make_scope(parallel_confs, target_parameters, workload_sizes)},
                  "output_initial.yaml", nqueued, repeats);
}

int main(int argc, char **argv) {
    args_parser parser(argc, argv);
    parser.add_vector<std::string>("workloads", "")
        .set_mode(args_parser::option::APPLY_DEFAULTS_ONLY_WHEN_MISSING);
    parser.add_vector<std::string>("scale", "")
        .set_mode(args_parser::option::APPLY_DEFAULTS_ONLY_WHEN_MISSING);
    parser.add_map("sizes", "", ',', ':');
    parser.add<int>("nqueued", 5);
    parser.add<int>("repeats", 10);
    parser.add<std::string>("driver", "imb_async");
    if (!parser.parse())
        return 1;
    int nqueued = parser.get<int>("nqueued");
    int repeats = parser.get<int>("repeats");
    auto driver = parser.get<std::string>("driver");
    if (driver == "imb_async") {
        parse_and_start<imb_async::traits>(parser, nqueued, repeats);
    } else {
        std::cout << "Unknown driver: " << driver << std::endl;
        return 1;
    }
    return 0;
}
