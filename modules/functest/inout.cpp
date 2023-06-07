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

#ifndef NATTEMPTS
#define NATTEMPTS 5   // was: 100 for Lom2 FIXME make it an external cmdline param
#endif

#ifndef SLEEPTIME
#define SLEEPTIME 10   // was: 100 for Lom2  FIXME make it an external cmdline param
#endif

#ifndef MISSING_FILES_FATAL // FIXME make it an external cmdline param
#define MISSING_FILES_FATAL 0
#endif


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

template <typename parallel_conf_t>
input_maker<parallel_conf_t>::input_maker(test_scope<functest::traits> &_scope)
    : scope(_scope) {
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
void input_maker<parallel_conf_t>::do_substs(const parallel_conf_t &pconf, std::string &filename) {
    helpers::subst(filename, "%WLD%", scope.workload_conf.first);
    helpers::subst(filename, "%CONF%", scope.workload_conf.second);
    helpers::subst(filename, "%WPRT%", scope.workparts[0].first);
    helpers::subst(filename, "%WPRT_PARAM%", std::to_string(scope.workparts[0].second));
    helpers::subst(filename, "%NP%", std::to_string(pconf.first));
    helpers::subst(filename, "%PPN%", std::to_string(pconf.second));
}

template <typename parallel_conf_t>
bool input_maker<parallel_conf_t>::file_exists(const parallel_conf_t &pconf, const std::string &filename_) {
    std::string filename = filename_;
	do_substs(pconf, filename);
	if (functest::traits::debug) {
		std::cout << ">> functest: prerequisite testing: trying to stat a file: " << filename << std::endl;
	}
	struct stat r;   
  	int retval = stat(filename.c_str(), &r);
    if (retval != 0 && functest::traits::debug) {
        perror(">> functest: stat");
    }
  	return (retval == 0);
}

template <typename parallel_conf_t>
bool input_maker<parallel_conf_t>::exec_shell_command(const parallel_conf_t &pconf, const test_item_t &testitem, 
                                                      const std::string &script, const std::vector<std::string> &exports,
                                                      std::string &result, int &status) {
	std::string command = script + " ";
    for (const auto &v : exports) {
        command += v + " ";
    }
	auto timeout = testitem.get_timeout(pconf.first, pconf.second);
	command += std::string("TIMEOUT=") + std::to_string(timeout) + " ";
    /*
    for (const auto &kv : testitem.base) {
        std::string replaced(kv.first);
        helpers::subst(replaced, "/", "_");
        command += std::string("TESTITEM__") + replaced + "=" + std::to_string(kv.second) + " ";
    }
    */
    char buffer[128]; // buffer to read the command's output
    FILE* pipe = popen(command.c_str(), "r"); // open a pipe to the command
    if (!pipe) {
        std::cerr << ">> functest: popen() failed: <" << command << ">" << std::endl;
        return false;
    }
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += std::string(buffer);
    }
    status = pclose(pipe);
    return true;
}

template <typename parallel_conf_t>
bool input_maker<parallel_conf_t>::check_prerequisites(const parallel_conf_t &pconf, execution_environment &env) {
    const auto &prereq = testitem.get_prerequisites();
    bool notexist = false;
    std::string file;
	for (const auto &elem : prereq) {
		file = elem.first;
		if (!file_exists(pconf, file)) {
            if (functest::traits::debug) {
    			std::cout << ">> functest: prerequisite testing: object doesn't exist: " << file << std::endl;
            }
			notexist = true;
			break;
		}
	}
	if (notexist) {
		env.holdover = true;
		env.holdover_reason = "Prerequisite object not found: " + file;
		return false;
	}
	env.holdover = false;
    env.holdover_reason = "";
	return true;
}


template <typename parallel_conf_t>
bool input_maker<parallel_conf_t>::make(const parallel_conf_t &pconf, execution_environment &env) {
	assert(scope.workparts.size() == 1);
    const auto &workload = scope.workload_conf.first;
	const auto &conf = scope.workload_conf.second;

    // If the skip flag is set, just return
    if (testitem.get_skip_flag(workload, pconf.first, pconf.second) || env.skip) {
        env.skip = true;
        return false;
    }

    // If prerequisites do not exist, set the holdover state and skip all further steps
	if (testitem.get_prerequisites_flag()) {
		if (!check_prerequisites(pconf, env))
			return false;
	}

    // Choose the expected psubmit options file name
    if (helpers::file_exists("./psubmit.opt") || conf == "X") {
        env.psubmit_options = "./psubmit.opt";
    } else { 
        env.psubmit_options = "./psubmit_" + conf + ".opt";
    }
    env.input_yaml = "./input_" + workload + ".yaml";

    // Create the set of environment variables for scripts that we are going to execute
    auto &exports = env.exports;
	for (const auto v : {"WLD", "CONF", "WPRT", "WPRT_PARAM", "NP", "PPN"}) {
		std::string s = std::string("MASSIVE_TESTS_TESTITEM_") + v + "=" + "%" + v + "%";
		do_substs(pconf, s);
		exports.push_back(s);
    } 

	// To form a command line we either execute the ./input_maker_cmdline.sh script or fill in some hard-coded values 
    // (hard coded values are deprecated)
    bool cmdline_requires_additional_filling = true;
    const std::string input_maker_script = "./input_maker_cmdline.sh";
    if (helpers::file_exists(input_maker_script) && helpers::file_is_exec(input_maker_script)) {
        cmdline_requires_additional_filling = false;
		int status = -1;
        bool result = exec_shell_command(pconf, testitem, input_maker_script + " 2>&1", env.exports, env.cmdline_args, status);
		if (!result) {
			env.skip = true;
            std::cout << "INPUT: input maker script execution failure" << std::endl;
			return false;
		}
		if (status != 0) {
			env.skip = true;
            std::cout << "INPUT: test item skipped: input maker script returned non-zero code and message: " << env.cmdline_args;
			return false;
		}
    } else {
        //--- cmdline hardcoded
        env.cmdline_args = load_key + " " + env.input_yaml;
        env.cmdline_args += std::string(" ") + result_key + std::string(" ") + " result.%PSUBMIT_JOBID%.yaml";
        if (conf_key != "") {
            env.cmdline_args += std::string(" ") + conf_key + std::string(" ") + conf;
        }
        if (timeout_key != "" && testitem.get_timeout(pconf.first, pconf.second)) {
            env.cmdline_args += std::string(" ") + timeout_key + std::string(" ") + 
                                std::to_string(testitem.get_timeout(pconf.first, pconf.second));
        }
        char *aux_opts;
        if ((aux_opts = getenv("MASSIVETEST_AUX_ARGS"))) {
            env.cmdline_args += " " + std::string(aux_opts);
        }
        //--- /cmdline hardcoded
    }

	// Make substitutions in preproc and postproc script names and args if they exist
    auto &pr = input_maker_base<parallel_conf_t>::preproc;
    pr = testitem.preproc;
	do_substs(pconf, pr);
	env.preproc = pr;

    auto &po = input_maker_base<parallel_conf_t>::postproc;
    po = testitem.postproc;
	do_substs(pconf, po);
	env.postproc = po;
	
    return cmdline_requires_additional_filling;
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
        if (!helpers::try_to_open_file<NATTEMPTS, SLEEPTIME>(in, infile)) {
#if MISSING_FILES_FATAL            
            std::cout << "OUTPUT: functest: stop processing: can't open input file: " << infile
                      << std::endl; 
            return;
#endif            
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
                if (!stream[section])
                    continue;
                auto comparator = testitem.get_comparator(parameter_code, pconf.first, pconf.second, indir);
                if (comparator->acquire_result_data_piece(stream, section, parameter))
                    vals[workpart].push_back(comparator);
            }
        }
        catch (std::runtime_error &ex) {
            std::cout << "OUTPUT: parse error on YAML file: " << infile << std::endl;
            values.clear();
            comment = std::string("dir=") + indir + "; parse error on YAML file: " + infile;
            break;
        }
    }
    // FIXME consider this test a command-line switchable option
#if MISSING_FILES_FATAL            
    assert(values.size() == attempts.size());
#endif
    attempts.resize(0);

    // Check if no actual data for any section/parameter pair is acquired
    if (status == status_t::P && values.size() == 0) {
        if (comment.size())
            comment = "No data found " + comment;
        else
            comment = "No data found";
        status = status_t::N;
    }

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
            status = v[0]->check_attempts_equality(v, comment);
            if (status != status_t::P)
                break;
            status = v[0]->compare(comment);
            if (status != status_t::P)
                break;
        }
    }

    // Make a result record for this testitem
    auto r = functest::traits::make_result(wconf, pconf, {"", ""}, workpart, status_to_string(status), comment);
    r->to_yaml(out);
    std::cout << "OUTPUT: functest: {" << wconf.first << "," << wconf.second << "}"
              << " on parallel conf: {" << pconf.first << "," << pconf.second << "} " << std::endl;
}

template class input_maker<functest::traits::parallel_conf_t>;
template class output_maker<functest::traits::parallel_conf_t>;

} // namespace functest
