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

#include "modules/imb_async/traits.h"
#include "modules/imb_async/inout.h"
#include "helpers.h"
#include "results.h"

namespace imb_async {

input_maker::input_maker(test_scope<imb_async::traits> &_scope)
    : was_written(false), scope(_scope) {}

void input_maker::write_out(const std::string &input_file_name) {
    if (was_written)
        return;
    std::ofstream ofs(input_file_name);
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    out << YAML::Flow;

    YAML_OUT("version", 0);

    // number of warmup cycles
    YAML_OUT("nwarmup", 10);

    // message sizes
    YAML_OUT_SEQ("len", scope.workload_sizes, { return v.first; });

    // ncycles
    YAML_OUT_SEQ("ncycles", scope.workload_sizes, { return v.second; });

    // REQUIRES CALIBRATION: calctime set for calc "workloads"
    char *calctime = NULL;
    if ((calctime = getenv("MASSIVETEST_CALCTIME"))) {
        auto vcalctime = helpers::vstr_to_vint(helpers::str_split(std::string(calctime), ','));
        assert(vcalctime.size() == scope.workload_sizes.size());
        YAML_OUT_SEQ("calctime", vcalctime, { return v; });
    }

    // REQUIRES CALIBRATION: number of calc cycles for 10 usec
    char *cper10usec = NULL;
    if ((cper10usec = getenv("MASSIVETEST_CPER10USEC"))) {
        auto icper10usec = std::stoi(cper10usec);
        assert(icper10usec > 0 && icper10usec < 1000);
        YAML_OUT("cper10usec", icper10usec);
    }

    // datatype
    YAML_OUT("datatype", "char");

    // bench names
    YAML_OUT_SEQ_NESTED("extra_args", scope.target_parameters, { return v.first; });

    out << YAML::EndMap;
    out << YAML::EndDoc;
    ofs << out.c_str();
    was_written = true;
}

void input_maker::make(std::string &input_yaml, std::string &psubmit_options, std::string &args) {
    input_yaml = "./input_" + std::to_string(scope.id) + ".yaml";
    psubmit_options = "./psubmit.opt";
    args = "-load " + input_yaml + " -output " + " benchmark.%PSUBMIT_JOBID%.yaml";
    char *aux_opts;
    if ((aux_opts = getenv("MASSIVETEST_AUX_ARGS"))) {
        args += " " + std::string(aux_opts);
    }
    write_out(input_yaml);
}

output_maker::output_maker(test_scope<imb_async::traits> &_scope, const std::string &_outfile)
    : scope(_scope), outfile(_outfile) {
    out << YAML::BeginSeq;
    out << YAML::Flow;
}

output_maker::~output_maker() {
    out << YAML::EndSeq;
    out << YAML::Newline;
    std::ofstream ofs(outfile, std::ios_base::out | std::ios_base::ate | std::ios_base::app);
    ofs << out.c_str();
}

void output_maker::make(std::vector<std::shared_ptr<process>> &attempts) {
    int n = -1, ppn = -1;
    using val_t = double;
    using vals_t = std::map<int, std::vector<val_t>>;
    std::map<std::string, vals_t> values;
    for (auto &proc : attempts) {
        int j = proc->jobid;
        if (n == -1)
            n = proc->n;
        if (ppn == -1)
            ppn = proc->ppn;
        assert(n == proc->n);
        assert(ppn == proc->ppn);
        std::string infile =
            "results." + std::to_string(j) + "/benchmark." + std::to_string(j) + ".yaml";
        std::ifstream in;
        if (!helpers::try_to_open_file<100, 100>(in, infile)) {
            // NOTE: commented out this return: lets handle missing input files as non-fatal case
            // std::cout << "OUTPUT: imb-async: stop processing: can't open input file: " << infile
            // << std::endl; return;
            std::cout << "OUTPUT: imb-async: warning: can't open input file: " << infile
                      << std::endl;
            continue;
        }
#ifdef DEBUG
        std::cout << ">> imb-async: input: reading " << infile << std::endl;
#endif
        auto stream = YAML::Load(in);
        for (auto &bp : scope.target_parameters) {
            std::string &benchmark = bp.first;
            std::string &parameter = bp.second;
            auto str_bp = helpers::conf_to_string(bp);
            auto &vals = values[str_bp];
            if (!stream[benchmark])
                continue;
            const auto &b = stream[benchmark].as<YAML::Node>();
            if (!b[parameter])
                continue;
            const auto &p = b[parameter].as<YAML::Node>();
            for (auto &it : scope.workload_sizes) {
                auto msglen = it.first;
                if (!p[msglen])
                    continue;
                vals[msglen].push_back(p[msglen].as<YAML::Node>().as<val_t>());
#ifdef DEBUG
                auto value = vals[msglen].back();
                std::cout << ">> imb-async: input: for msglen=" << msglen << " found val=" << value
                          << std::endl;
#endif
            }
        }
    }
    // NOTE: commented out this assert: lets handle missing input files as non-fatal case
    // assert(values.size() == attempts.size());
    attempts.resize(0);
    int nresults = 0;
    for (auto &it : values) {
        auto bp = helpers::str_split(it.first, '+');
        assert(bp.size() == 2);
        auto benchmark = bp[0];
        auto parameter = bp[1];
        auto &vals = it.second;
#ifdef DEBUG
        std::cout << ">> imb-async: output: benchmark=" << benchmark << " parameter=" << parameter
                  << std::endl;
#endif
        for (auto &szs : scope.workload_sizes) {
            auto msglen = szs.first;
            auto niter = szs.second;
            auto &v = vals[msglen];
            if (v.size() == 0) {
#ifdef DEBUG
                std::cout << ">> imb-async: output: nothing found for msglen=" << msglen
                          << std::endl;
#endif
                continue;
            }
            val_t result_val;
            if (v.size() == 1) {
                result_val = v[0];
            } else {
                sort(v.begin(), v.end());
                result_val = helpers::average(v);
            }
            imb_async::traits traits;
            auto r =
                traits.make_result({n, ppn}, {benchmark, parameter}, {msglen, niter}, result_val);
            r->to_yaml(out);
#ifdef DEBUG
            std::cout << ">> imb-async: output: result written: " << r->get_index() << std::endl;
#endif
            nresults++;
        }
    }
    std::cout << "OUTPUT: imb-async: {" << n << "," << ppn << "} " << nresults
              << " resulting items registered" << std::endl;
}

} // namespace imb_async
