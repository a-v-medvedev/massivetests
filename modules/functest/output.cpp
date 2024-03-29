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
#include <regex>
#include <unistd.h>
#include <memory>
#include <assert.h>
#include <cmath>

#include <sys/types.h>
#include <sys/stat.h>

#include "modules/functest/traits.h"
#include "modules/functest/inout.h"

#include "helpers.h"
#include "results.h"

namespace functest {

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

static status_t check_if_failed(const std::string &s, const std::string &jid) {
    std::string stackfile = s + "/stacktrace." + jid;
    std::ifstream in;
	if (helpers::try_to_open_file(in, stackfile, functest::traits::open_outfile_nattempts, functest::traits::open_outfile_sleeptime)) {
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

template <typename parallel_conf_t>
output_maker<parallel_conf_t>::output_maker(test_scope<functest::traits> &_scope, const std::string &_outfile)
    : scope(_scope), outfile(_outfile) {
    out << YAML::BeginSeq;
    out << YAML::Flow;
    assert(scope.workparts.size() == 1);
    const auto workload_conf = scope.workload_conf;
    std::string item;
    if (workload_conf.second == "X") {
        item = workload_conf.first + "/" + scope.workparts[0].first;
    } else {
        item = workload_conf.first + "/" + workload_conf.second + "/" + scope.workparts[0].first;
    }
    testitem.load(item);
}

template <typename parallel_conf_t>
output_maker<parallel_conf_t>::~output_maker() {
    out << YAML::EndSeq;
    out << YAML::Newline;
    std::ofstream ofs(outfile, std::ios_base::out | std::ios_base::ate | std::ios_base::app);
    ofs << out.c_str();
}

template <typename parallel_conf_t>
void output_maker<parallel_conf_t>::make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts) {
    parallel_conf_t pconf;
	const auto wconf = scope.workload_conf;
	const auto workpart = scope.workparts[0];
    using vals_t = std::map<decltype(workpart), std::vector<std::shared_ptr<comparator_t>>>;
    std::map<std::string, vals_t> values;
    status_t status = status_t::P;
    std::string comment;

    for (auto &proc : attempts) {
        
        // Job sumbitting failure case -- immediate stop
        int j = proc->jobid;
        if (j == -1 && !proc->skipped) {
            std::cout << "OUTPUT: FATAL: failure in submitting job via psubmit. Output:\n-----" << std::endl;
            std::cout << proc->full_output << std::endl;
            assert(0 && "psubmit starting problem");
        }
        pconf = proc->pconf;

        // Skipped item case
        if (proc->skipped) {
            if (proc->env.holdover_reason != "") {
                comment = proc->env.holdover_reason;
            } else {
                comment = "Marked as skipped";
            }
            status = status_t::S;
            break;
        }
        std::string indir = std::string("results.") + std::to_string(j);

        // Application return value check
        if (proc->retval) {
            comment = std::string("Non-zero return code: ") + std::to_string(proc->retval);
            comment += std::string(" dir=") + indir;
            status = status_t::N;
            struct stat s;
            bool ok = false;
            int r = stat(indir.c_str(), &s);
            if (!r && ((s.st_mode & S_IFDIR) == S_IFDIR)) {
                ok = true;
            }
            if (!ok) {
                std::cout << "OUTPUT: functest: can't open directory: " << indir
                          << std::endl;
                return;
            }
            break;
        }

        // TACE failure cases (Timeout, Assert, Crash, Exception) check
        std::string infile = indir + "/result." + std::to_string(j) + ".yaml";
        auto st = check_if_failed(indir, std::to_string(j));
        if (st != status_t::P) {
            comment = std::string("dir=") + indir;
            status = st;
            break;
        }

        // Application result YAML -- check if it exists
        std::ifstream in;
        if (!helpers::try_to_open_file(in, infile, functest::traits::open_outfile_nattempts, functest::traits::open_outfile_sleeptime)) {
            if (functest::traits::missing_files_fatal) {
                std::cout << "OUTPUT: functest: stop processing: can't open input file: " << infile
                          << std::endl; 
                return;
            }
            std::cout << "OUTPUT: functest: warning: can't open input file: " << infile
                      << std::endl;
            continue;
        }
        if (functest::traits::debug) {
            std::cout << ">> functest: input: reading " << infile << std::endl;
        }

        // Read all the data from application result YAML
        try {
            auto stream = YAML::Load(in);
            auto tps = testitem.update_target_parameters(scope.target_parameters);

            // For every expected section/parameter pair create a specific comparator
            for (auto &sp : tps) {
                std::string &section = sp.first;
                std::string &parameter = sp.second;
                auto parameter_code = functest::traits::target_parameter_to_string(sp);
                auto &vals = values[parameter_code];
                YAML::Node node = helpers::advance_yaml_node(stream, section);
                if (!node)
                    continue;
                auto comparator = testitem.get_comparator(parameter_code, functest::traits::parallel_conf_to_string(pconf), indir);
                if (comparator->acquire_result_data_piece(node, parameter))
                    vals[workpart].push_back(comparator);
            }
        }
        catch (std::runtime_error &ex) {
            std::cout << "OUTPUT: parse error on YAML file: " << infile << ": " << ex.what() << std::endl;
            values.clear();
            comment = std::string("dir=") + indir + "; parse error on YAML file: " + infile + ": " + ex.what();
            break;
        }
    }

    if (functest::traits::missing_files_fatal) {
        assert(values.size() == attempts.size());
    }

    attempts.resize(0);

    // Check if no actual data for any section/parameter pair is acquired
    if (status == status_t::P && values.size() == 0) {
        if (comment.size())
            comment = "No data found " + comment;
        else
            comment = "No data found";
        status = status_t::N;
    }

    std::map<std::string, std::string> auxilary;
    // For each section/parameter pair -- run a comparator
    if (status == status_t::P) {
        for (auto &it : values) {
            std::string parameter_code = it.first;
            auto &vals = it.second;
            if (functest::traits::debug) {
                std::cout << ">> functest: output: section/parameter=" << parameter_code << std::endl;
            }
            auto &v = vals[workpart];
            if (v.size() == 0) {
                if (functest::traits::debug) {
                    std::cout << ">> functest: output: nothing found for section/parameter: " << parameter_code << std::endl;
                }
                comment = std::string("No data for section/parameter=") + parameter_code;
                status = status_t::N;
                break;
            }
            status = v[0]->handle_attempts(v, comment);
            if (status != status_t::P)
                break;
            status = v[0]->compare(comment, auxilary);
            if (status != status_t::P)
                break;
        }
    }

    // Make a result record for this testitem
    auto r = functest::traits::make_result(wconf, pconf, {"", ""}, workpart, status_to_string(status), comment, auxilary);
    r->to_yaml(out);
    std::cout << "OUTPUT: functest: {" << wconf.first << "," << wconf.second << "}"
              << " on parallel conf: {" << functest::traits::parallel_conf_to_string(pconf) << "} " << std::endl;
}

template class output_maker<functest::traits::parallel_conf_t>;

} // namespace functest
