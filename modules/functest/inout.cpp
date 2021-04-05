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

#include <sys/types.h>
#include <sys/stat.h>

#include "modules/functest/traits.h"
#include "modules/functest/inout.h"
#include "modules/functest/inout_teststub.h"
#include "modules/functest/inout_xamg.h"

#include "helpers.h"
#include "results.h"

#ifndef NATTEMPTS
#define NATTEMPTS 5   // was: 100 for Lom2
#endif

#ifndef SLEEPTIME
#define SLEEPTIME 10   // was: 100 for Lom2
#endif

#ifndef MISSING_FILES_FATAL
#define MISSING_FILES_FATAL 0
#endif


namespace functest {

enum status_t { P=0, F=1, N=2, S=3, T=4, A=5, C=6, E=7 };

static const std::string status_to_string(status_t st) {
    switch (st) {
        case status_t::P: return "P";
        case status_t::F: return "F";
        case status_t::N: return "N";
        case status_t::S: return "S";
        case status_t::T: return "T";
        case status_t::A: return "A";
        case status_t::C: return "C";
        case status_t::E: return "E";
        default: return "?";
    }
    return "?";
}

input_maker::input_maker(test_scope<functest::traits> &_scope)
    : scope(_scope) {
    assert(scope.workload_sizes.size() == 1);
    const auto workload_conf = scope.workload_conf;
    std::string item;
    if (workload_conf.second == "X") {
        item = workload_conf.first + "/" + scope.workload_sizes[0].first;
    } else {
        item = workload_conf.first + "/" + workload_conf.second + "/" + scope.workload_sizes[0].first;
    }
	testitem.load(item);
}

/*
void input_maker::write_out(const std::string &input_file_name) {
    (void)input_file_name;
    if (was_written)
        return;
   
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
    
    was_written = true;
}
*/

void input_maker::make(std::string &input_yaml, std::string &psubmit_options, std::string &args) {
	assert(scope.workload_sizes.size() == 1);
    if (testitem.skip) {
        psubmit_options = "";
        args = "";
        return;
    }
	const auto &workload = scope.workload_conf.first;
	const auto &conf = scope.workload_conf.second;
    if (conf == "X") {
        psubmit_options = "./psubmit.opt";
    } else {
        psubmit_options = "./psubmit_" + conf + ".opt";
    }
    input_yaml = "./input_" + workload + ".yaml";
    args = load_key + " " + input_yaml;
    args += std::string(" ") + result_key + std::string(" ") + " result.%PSUBMIT_JOBID%.yaml";
    if (conf_key != "") {
        args += std::string(" ") + conf_key + " " + conf;
    }
    if (timeout_key != "" && testitem.timeout) {
        args += std::string(" -timeout ") + std::to_string(testitem.timeout);
    }

    char *aux_opts;
    if ((aux_opts = getenv("MASSIVETEST_AUX_ARGS"))) {
        args += " " + std::string(aux_opts);
    }
}

output_maker::output_maker(test_scope<functest::traits> &_scope, const std::string &_outfile)
    : scope(_scope), outfile(_outfile) {
    out << YAML::BeginSeq;
    out << YAML::Flow;
    assert(scope.workload_sizes.size() == 1);
    const auto workload_conf = scope.workload_conf;
    std::string item;
    if (workload_conf.second == "X") {
        item = workload_conf.first + "/" + scope.workload_sizes[0].first;
    } else {
        item = workload_conf.first + "/" + workload_conf.second + "/" + scope.workload_sizes[0].first;
    }
    testitem.load(item);
}

output_maker::~output_maker() {
    out << YAML::EndSeq;
    out << YAML::Newline;
    std::ofstream ofs(outfile, std::ios_base::out | std::ios_base::ate | std::ios_base::app);
    ofs << out.c_str();
}

status_t check_if_failed(const std::string &s, const std::string &jid) {
    std::string stackfile = s + "/stacktrace." + jid;
    std::ifstream in;
	if (helpers::try_to_open_file<NATTEMPTS, SLEEPTIME>(in, stackfile)) {
		std::cout << "OUTPUT: found stackfile: " << stackfile << std::endl;
		std::string status;
		std::getline(in, status);
        auto st = helpers::str_split(status, ' ');
        if (st[0] == ">>" && st[1] == "STATUS:") {
		    char c = st[2][0];
			if (c == 'T') return status_t::T;
			if (c == 'A') return status_t::A;
			if (c == 'C') return status_t::C;
			if (c == 'E') return status_t::E;
		}
	}
    return status_t::P;
}

void output_maker::make(std::vector<std::shared_ptr<process>> &attempts) {
    functest::traits traits;
    int n = -1, ppn = -1;
	const auto wconf = scope.workload_conf;
	const auto size = scope.workload_sizes[0];
    using val_t = double;
    using vals_t = std::map<decltype(size), std::vector<val_t>>;
    std::map<std::string, vals_t> values;
    status_t status = status_t::P;
    std::string comment;
    for (auto &proc : attempts) {
        int j = proc->jobid;
        if (j == -1) {
            std::cout << "OUTPUT: FATAL: failure in submitting job via psubmit. Output:\n-----" << std::endl;
            std::cout << proc->full_output << std::endl;
            return;
        }
        if (n == -1)
            n = proc->n;
        if (ppn == -1)
            ppn = proc->ppn;
        assert(n == proc->n);
        assert(ppn == proc->ppn);
        if (proc->skipped) {
            comment = "Marked as skipped";
            status = status_t::S;
            break;
        }
        if (proc->retval) {
            comment = std::string("Non-zero return code: ") + std::to_string(proc->retval);
            status = status_t::N;
            std::string dir = "results." + std::to_string(j);
            struct stat s;
            bool ok = false;
            int r = stat(dir.c_str(), &s);
            if (!r && ((s.st_mode & S_IFDIR) == S_IFDIR)) {
                ok = true;
            }
            if (!ok) {
                std::cout << "OUTPUT: functest: can't open directory: " << dir
                          << std::endl;
                return;
            }
            break;
        }
        std::string infile =
            "results." + std::to_string(j) + "/result." + std::to_string(j) + ".yaml";
        auto st = check_if_failed("results." + std::to_string(j), std::to_string(j));
        if (st != status_t::P) {
            comment = std::string("dir=results.") + std::to_string(j);
            status = st;
            break;
        }
        std::ifstream in;
        if (!helpers::try_to_open_file<NATTEMPTS, SLEEPTIME>(in, infile)) {
#if MISSING_FILES_FATAL            
            // NOTE: commented out this return: let us handle missing input files as a non-fatal case
            std::cout << "OUTPUT: functest: stop processing: can't open input file: " << infile
                      << std::endl; 
            return;
#endif            
            std::cout << "OUTPUT: functest: warning: can't open input file: " << infile
                      << std::endl;
            continue;
        }
#ifdef DEBUG
        std::cout << ">> functest: input: reading " << infile << std::endl;
#endif
        auto stream = YAML::Load(in);
        auto tps = testitem.update_target_parameters(scope.target_parameters);
        for (auto &sp : tps) {
            std::string &section = sp.first;
            std::string &parameter = sp.second;
            auto str_sp = traits.target_parameter_to_string(sp);
            auto &vals = values[str_sp];
            if (!stream[section])
                continue;
            const auto &sec = stream[section].as<YAML::Node>();
            if (parameter.find("[") != std::string::npos && 
                parameter.find("]") != std::string::npos) {
                auto pv = helpers::str_split(parameter, '[');
                assert(pv.size() == 2);
                auto idxv = helpers::str_split(pv[1], ']');
                //std::cout << ">> OUTPUT: parameter=" << parameter << std::endl;
                assert(idxv.size() == 1);
                auto p = pv[0];
                auto idx = idxv[0];
                //std::cout << ">> OUTPUT: parameter=" << parameter << " " << p << "," << idx <<  std::endl;
                size_t i = std::stol(idx);
                if (!sec[p])
                    continue;
                const auto &pn = sec[p].as<YAML::Node>();
                size_t n = 0;
                for (YAML::const_iterator it = pn.begin(); it != pn.end(); ++it) {
                    if (i == n++) {
                        vals[size].push_back(it->as<val_t>());
                        break;
                    }
                }
            } else {
                if (!sec[parameter])
                    continue;
                const auto &p = sec[parameter].as<YAML::Node>();
                vals[size].push_back(p.as<val_t>());
            }
        }
    }
    // NOTE: commented out this assert: lets handle missing input files as non-fatal case
    // assert(values.size() == attempts.size());
    attempts.resize(0);
    int nresults = 0;
    if (status == status_t::P && values.size() == 0) {
        comment = "No data found";
        status = status_t::N;
    }
    if (status == status_t::P) {
        for (auto &it : values) {
            auto sp = helpers::str_split(it.first, '+');
            assert(sp.size() == 2);
            auto section = sp[0];
            auto parameter = sp[1];
            auto &vals = it.second;
#ifdef DEBUG
            std::cout << ">> functest: output: section=" << section << " parameter=" << parameter
                      << std::endl;
#endif
            auto &v = vals[size];
            if (v.size() == 0) {
#ifdef DEBUG
                std::cout << ">> functest: output: nothing found for section/parameter: " << it.first
                          << std::endl;
#endif
                comment = std::string("No data for par=") + it.first;
                status = status_t::N;
                break;
            }
            val_t result_val = 0.0;
            if (v.size() == 1) {
                result_val = v[0];
            } else {
                sort(v.begin(), v.end());
                if (v.front() != v.back()) {
#ifdef DEBUG
                    std::cout << ">> functest: v.front() != v.back(). ATTEMPTS COMPARISON FAILED!" << std::endl;
#endif
                    comment = std::string("Attempts comparison failed par=") + it.first + std::string(" diff=") + std::to_string(fabs(v.front() - v.back())); 
                    status = status_t::F;
                    break;
                }
                result_val = v[0];
            }
            double diff = fabs(result_val - testitem.base[section + "/" + parameter]);
            if (diff > 1e-6) {
#ifdef DEBUG
                std::cout << ">> functest: diff > 1e-6. GOLD VALUE COMPARISON FAILED!" << std::endl;
#endif
                comment = std::string("Gold value comparison failed par=") + it.first + std::string(" diff=") + std::to_string(diff); 
                status = status_t::F;
                break;
            }
        }
    }
    auto r = traits.make_result(wconf, {n, ppn}, {"", ""}, size, status_to_string(status), comment);
    r->to_yaml(out);
    nresults++;
    std::cout << "OUTPUT: functest: {" << wconf.first << "," << wconf.second << "}"
              << " on parallel conf: {" << n << "," << ppn << "} " << nresults
              << " resulting items registered" << std::endl;
}

} // namespace functest
