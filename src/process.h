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

#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "helpers.h"

struct execution_environment {
    std::string input_yaml;
    std::string psubmit_options;
    std::string cmdline_args;
    std::string preproc, postproc;
    std::vector<std::string> exports;
    bool skip = false;
    bool holdover = false;
    std::string holdover_reason;
    std::string executable = "psubmit.sh";
    template <typename parallel_conf_t> 
    void exec(const parallel_conf_t &pconf) const {
        std::cout << "execlp: " << executable << " " << executable << " "
                  << "-n"
                  << " " << std::to_string(pconf.nnodes) << " "
                  << "-o"
                  << " " << psubmit_options << " "
                  << "-a"
                  << " " << cmdline_args << " " 
                  << "-b"
                  << " " << preproc << " "
                  << "-f"
                  << " " << postproc << " ";
        if (pconf.ppn) {
            std::cout  << "-p"
                       << " " << std::to_string(pconf.ppn) << " ";
        }
        if (pconf.nth) {
            std::cout  << "-t"
                       << " " << std::to_string(pconf.nth) << " ";
        }
        std::cout << std::endl;

        for (const auto &v : exports) {
            auto kv = helpers::str_split(v, '=');
            setenv(kv[0].c_str(), kv[1].c_str(), 1);
        }
		auto full_executable = helpers::which(executable);
		if (full_executable.empty()) {
			throw std::runtime_error(std::string("exec: can't find file or it has no execution permissions: " + executable));
		}
        char *argv[18];
        argv[0] = strdup(executable.c_str());
        argv[1] = strdup("-n"); argv[2] = strdup(std::to_string(pconf.nnodes).c_str());
        argv[3] = strdup("-o"); argv[4] = strdup(psubmit_options.c_str());
        argv[5] = strdup("-a"); argv[6] = strdup(cmdline_args.c_str());
        argv[7] = strdup("-b"); argv[8] = strdup(preproc.c_str());
        argv[9] = strdup("-f"); argv[10]= strdup(postproc.c_str());
        size_t n = 11;
        if (pconf.ppn) {
            argv[n++] = strdup("-p");
            argv[n++] = strdup(std::to_string(pconf.ppn).c_str());
        }
        if (pconf.nth) {
            argv[n++] = strdup("-t");
            argv[n++] = strdup(std::to_string(pconf.nth).c_str());
        }
        argv[n] = nullptr;
        execvp(full_executable.c_str(), argv);
    }
    std::string to_string() {
        std::stringstream ss;
        if (!input_yaml.empty()) {  // FIXME input_yaml field seems to be redundant
            ss << "input=" << input_yaml << " ";
        }
        ss << "psubmit_options=" << psubmit_options << " ";
        ss << "args={" << cmdline_args << "}";
        return ss.str();
    }
    void parse_line(const std::string &line_from_child, std::string &state, int &jobid) {
        {
            char a1[128], a2[128], a3[128];
            int n = sscanf(line_from_child.c_str(), "%s %s %s\n", a1, a2, a3);
            if (n == 3 && !strcmp(a1, "Job") && !strcmp(a2, "status:")) {
                state = a3;
            }
        }

        {
            char a1[128], a2[128];
            int a3;
            int n = sscanf(line_from_child.c_str(), "%s %s %d\n", a1, a2, &a3);
            if (n == 3 && !strcmp(a1, "Job") && !strcmp(a2, "ID")) {
                jobid = a3;
            }
        }
    }
};

template <typename parallel_conf_t>
struct process {
    parallel_conf_t pconf;
    execution_environment env;
    pid_t pid = 0;
    int jobid = -1;
    int retval = 0;
    bool skipped = false;
    std::string state = "NONE";
    int pipe_fd[2];
    FILE *outfp = nullptr;
    std::shared_ptr<input_maker_base<parallel_conf_t>> im;
    std::shared_ptr<output_maker_base<parallel_conf_t>> om;
    std::string full_output;

    process(parallel_conf_t _pconf, std::shared_ptr<input_maker_base<parallel_conf_t>> _input_maker,
            std::shared_ptr<output_maker_base<parallel_conf_t>> _output_maker)
        : pconf(_pconf), im(_input_maker), om(_output_maker) {}

    void create_environment() {
		im->make(pconf, env);
    }
    void start() {
        if (env.skip) {
            state = "FINISHED";
            pid = 0;
            skipped = true;
            return;
        }
        assert(!env.holdover);
        int ret = pipe(pipe_fd);
        if (ret != 0) {
            throw std::runtime_error(std::string("start: pipe error: ") + strerror(errno)); 
        }
        pid = fork();
        if (pid == 0) { // Child
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            dup2(pipe_fd[1], STDERR_FILENO);
            env.exec<parallel_conf_t>(pconf);
            std::cout << "execlp failed! " << strerror(errno) << std::endl;
            exit(1);
        }
        // Parent
        close(pipe_fd[1]);
        int f = fcntl(pipe_fd[0], F_GETFL, 0) | O_NONBLOCK;
        fcntl(pipe_fd[0], F_SETFL, f);
        outfp = fdopen(pipe_fd[0], "r");
    }

    bool update_state() {
        if (0 == pid)
            return true;
        if ("FINISHED" == state)
            return true;
        char line_from_child[1024];
        char *s = fgets(line_from_child, sizeof(line_from_child), outfp);
        if (NULL == s) {
            if (EWOULDBLOCK == errno) {
                if (wait_non_blocking()) {
                    return true;
                }
                return false;
            }
            this->wait();
            return true;
        }
        full_output += s;
#ifdef DEBUG  // FIXME consider making this output a command-line switchable option
        std::cout << pid << ": " << s; // no endline, it is already there
#endif
        env.parse_line(std::string(line_from_child), state, jobid);
        return false;
    }

    void kill() {
        if (!pid)
            return;
        ::kill(pid, SIGTERM);
        this->wait();
    }
    
    void cleanup() {
        if (!pid)
            return;
        state = "FINISHED";
        pid = 0;
        if (outfp != nullptr) {
            fclose(outfp);
            outfp = nullptr;
        }
    }

    void wait() {
        if (!pid)
            return;
        int status;
        waitpid(pid, &status, 0);
        if (!(WIFEXITED(status))) {
            retval = 1;
        } else {
            retval = WEXITSTATUS(status);
        }
        cleanup();
    }

    bool wait_non_blocking() {
        if (!pid)
            return false;
        int status;
        int res = waitpid(pid, &status, WNOHANG);
        if (res == 0)
            return false;
        if (!(WIFEXITED(status))) {
            retval = 1;  
        } else {
            retval = WEXITSTATUS(status);
        }
        cleanup();
        return true;
    }

    ~process() { this->kill(); }
};
