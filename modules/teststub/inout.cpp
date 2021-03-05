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

#include "modules/teststub/traits.h"
#include "modules/teststub/inout.h"
#include "helpers.h"
#include "results.h"

#ifndef NATTEMPTS
#define NATTEMPTS 1   // was: 100 for Lom2
#endif

#ifndef SLEEPTIME
#define SLEEPTIME 1   // was: 100 for Lom2
#endif



namespace teststub {

input_maker::input_maker(test_scope<teststub::traits> &_scope)
    : was_written(false), scope(_scope) {
    assert(scope.workload_sizes.size() == 1);
    const auto workload = scope.workload_sizes[0];
	testitem.load(workload.as_string());
}

void input_maker::write_out(const std::string &input_file_name) {
    (void)input_file_name;
    if (was_written)
        return;
/*    
    std::ofstream ofs(input_file_name);
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    out << YAML::Flow;

    YAML_OUT("version", 0);
    YAML_OUT("parameter", 10);

    out << YAML::EndMap;
    out << YAML::EndDoc;
    ofs << out.c_str();
*/    
    was_written = true;
}

void input_maker::make(std::string &input_yaml, std::string &psubmit_options, std::string &args) {
	assert(scope.workload_sizes.size() == 1);
	const auto workload = scope.workload_sizes[0];
    input_yaml = "./input_" + workload.workload + ".yaml";
    psubmit_options = "./psubmit.opt";
    args = "-load " + input_yaml + " -output " + " result.%PSUBMIT_JOBID%.yaml";
    char *aux_opts;
    if ((aux_opts = getenv("MASSIVETEST_AUX_ARGS"))) {
        args += " " + std::string(aux_opts);
    }
    write_out(input_yaml);
}

output_maker::output_maker(test_scope<teststub::traits> &_scope, const std::string &_outfile)
    : scope(_scope), outfile(_outfile) {
    out << YAML::BeginSeq;
    out << YAML::Flow;
    assert(scope.workload_sizes.size() == 1);
    const auto workload = scope.workload_sizes[0];
	testitem.load(workload.as_string());
}

output_maker::~output_maker() {
    out << YAML::EndSeq;
    out << YAML::Newline;
    std::ofstream ofs(outfile, std::ios_base::out | std::ios_base::ate | std::ios_base::app);
    ofs << out.c_str();
}

int check_if_failed(const std::string &s) {
    (void)s;
    return 0;
}

void output_maker::make(std::vector<std::shared_ptr<process>> &attempts) {
    teststub::traits traits;
    int n = -1, ppn = -1;
	const auto workload = scope.workload_sizes[0];
    using val_t = double;
    using vals_t = std::map<decltype(workload), std::vector<val_t>>;
    std::map<std::string, vals_t> values;
    int status = 0;
    for (auto &proc : attempts) {
        int j = proc->jobid;
        if (n == -1)
            n = proc->n;
        if (ppn == -1)
            ppn = proc->ppn;
        assert(n == proc->n);
        assert(ppn == proc->ppn);
        if (proc->retval) {
            status = 2;
            break;
        }
        std::string infile =
            "results." + std::to_string(j) + "/output." + std::to_string(j) + ".yaml";
        auto st = check_if_failed("results." + std::to_string(j));
        if (st) {
            status = st;
            break;
        }
        std::ifstream in;
        if (!helpers::try_to_open_file<NATTEMPTS, SLEEPTIME>(in, infile)) {
            // NOTE: commented out this return: let us handle missing input files as a non-fatal case
            // std::cout << "OUTPUT: teststub: stop processing: can't open input file: " << infile
            // << std::endl; return;
            std::cout << "OUTPUT: teststub: warning: can't open input file: " << infile
                      << std::endl;
            continue;
        }
#ifdef DEBUG
        std::cout << ">> teststub: input: reading " << infile << std::endl;
#endif
        auto stream = YAML::Load(in);
        for (auto &sp : scope.target_parameters) {
            std::string &section = sp.first;
            std::string &parameter = sp.second;
            auto str_sp = traits.target_parameter_to_string(sp);
            auto &vals = values[str_sp];
            if (!stream[section])
                continue;
            const auto &sec = stream[section].as<YAML::Node>();
            if (!sec[parameter])
                continue;
            const auto &p = sec[parameter].as<YAML::Node>();
            vals[workload].push_back(p.as<val_t>());
        }
    }
    // NOTE: commented out this assert: lets handle missing input files as non-fatal case
    // assert(values.size() == attempts.size());
    attempts.resize(0);
    int nresults = 0;
    if (!status) {
        for (auto &it : values) {
            auto sp = helpers::str_split(it.first, '+');
            assert(sp.size() == 2);
            auto section = sp[0];
            auto parameter = sp[1];
            auto &vals = it.second;
#ifdef DEBUG
            std::cout << ">> teststub: output: section=" << section << " parameter=" << parameter
                      << std::endl;
#endif
            auto &v = vals[workload];
            if (v.size() == 0) {
#ifdef DEBUG
                std::cout << ">> teststub: output: nothing found for section/parameter: " << it.first
                          << std::endl;
#endif
                status = 1;
                break;
            }
            val_t result_val = 0.0;
            if (v.size() == 1) {
                result_val = v[0];
            } else {
                sort(v.begin(), v.end());
                if (v.front() != v.back()) {
                    status = 1;
                    break;
                }
                result_val = v[0];
            }
            if (result_val != testitem.base[section + "/" + parameter]) {
                status = 1;
                break;
            }
        }
    }
    auto r = traits.make_result({n, ppn}, {"-", "-"}, workload, status);
    r->to_yaml(out);
    nresults++;
    std::cout << "OUTPUT: teststub: {" << n << "," << ppn << "} " << nresults
              << " resulting items registered" << std::endl;
}

} // namespace teststub
