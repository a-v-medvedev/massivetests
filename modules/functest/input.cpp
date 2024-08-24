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

namespace functest {

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
    helpers::subst(filename, "%NNODES%", std::to_string(pconf.nnodes));
    helpers::subst(filename, "%PPN%", std::to_string(pconf.ppn));
    helpers::subst(filename, "%NTH%", std::to_string(pconf.nth));
    helpers::subst(filename, "%NP%", std::to_string(pconf.nnodes*pconf.ppn));
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
	auto timeout = testitem.get_timeout(functest::traits::parallel_conf_to_string(pconf));
	command += std::string("TIMEOUT=") + std::to_string(timeout) + " ";
#if 0 // This code is specific for basic comparators, cannot handle complex ones this way    
    for (const auto &kv : testitem.base) {
        std::string replaced(kv.first);
        helpers::subst(replaced, "/", "_");
        command += std::string("TESTITEM__") + replaced + "=" + std::to_string(kv.second) + " ";
    }
#endif    
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
    if (testitem.get_skip_flag(functest::traits::parallel_conf_to_string(pconf)) || env.skip) {
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
	for (const auto v : {"WLD", "CONF", "WPRT", "WPRT_PARAM", "NNODES", "PPN", "NTH", "NP"}) {
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
        //--- cmdline hardcoded (DEPRECATED)
        env.cmdline_args = load_key + " " + env.input_yaml;
        env.cmdline_args += std::string(" ") + result_key + std::string(" ") + " result.%PSUBMIT_JOBID%.yaml";
        if (conf_key != "") {
            env.cmdline_args += std::string(" ") + conf_key + std::string(" ") + conf;
        }
        if (timeout_key != "" && testitem.get_timeout(functest::traits::parallel_conf_to_string(pconf))) {
            env.cmdline_args += std::string(" ") + timeout_key + std::string(" ") + 
                                std::to_string(testitem.get_timeout(functest::traits::parallel_conf_to_string(pconf)));
        }
        char *aux_opts;
        if ((aux_opts = getenv("MASSIVETEST_AUX_ARGS"))) {
            env.cmdline_args += " " + std::string(aux_opts);
        }
        //--- /cmdline hardcoded (DEPRECATED)
    }

	// Make substitutions in preproc and postproc script names and args if they exist
    auto &pr = input_maker_base<parallel_conf_t>::preproc;
    pr = testitem.get_preproc();
	do_substs(pconf, pr);
	env.preproc = pr;

    auto &po = input_maker_base<parallel_conf_t>::postproc;
    po = testitem.get_postproc();
	do_substs(pconf, po);
	env.postproc = po;
	
    return cmdline_requires_additional_filling;
}

template class input_maker<functest::traits::parallel_conf_t>;

} // namespace functest
