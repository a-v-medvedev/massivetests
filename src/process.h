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
#include <stdio.h>
#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

struct process {
    const std::string executable = "psubmit.sh";
    using parallel_size_t = std::pair<int, int>;
    int n, ppn;
    pid_t pid = 0;
    int jobid = 0;
    int retval = 0;
    bool skipped = false;
    std::string state = "NONE";
    int pipe_fd[2];
    FILE *outfp = nullptr;
    std::shared_ptr<input_maker_base> im;
    std::shared_ptr<output_maker_base> om;

    process(parallel_size_t s, std::shared_ptr<input_maker_base> _input_maker,
            std::shared_ptr<output_maker_base> _output_maker)
        : n(s.first), ppn(s.second), im(_input_maker), om(_output_maker) {}

    void start(const std::string &input_yaml, const std::string &psubmit_options,
               const std::string &args) {
        if (psubmit_options == "" && args == "") {
            state = "FINISHED";
            pid = 0;
            skipped = true;
            return;
        }
        (void)input_yaml;
        pipe(pipe_fd);
        pid = fork();
        if (pid == 0) { // Child
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            dup2(pipe_fd[1], STDERR_FILENO);
            std::cout << "execlp: " << executable << " " << executable << " "
                      << "-n"
                      << " " << std::to_string(n) << " "
                      << "-p"
                      << " " << std::to_string(ppn) << " "
                      << "-o"
                      << " " << psubmit_options << " "
                      << "-a"
                      << " " << args << std::endl;
            execlp(executable.c_str(), executable.c_str(), "-n", std::to_string(n).c_str(), "-p",
                   std::to_string(ppn).c_str(), "-o", psubmit_options.c_str(), "-a", args.c_str(),
                   (char *)NULL);
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
#ifdef DEBUG
        std::cout << pid << ": " << s; // no endline, it is already there
#endif

        {
            char a1[128], a2[128], a3[128];
            int n = sscanf(line_from_child, "%s %s %s\n", a1, a2, a3);
            if (n == 3 && !strcmp(a1, "Job") && !strcmp(a2, "status:"))
                state = a3;
        }

        {
            char a1[128], a2[128];
            int a3;
            int n = sscanf(line_from_child, "%s %s %d\n", a1, a2, &a3);
            if (n == 3 && !strcmp(a1, "Job") && !strcmp(a2, "ID")) {
                jobid = a3;
            }
        }
        return false;
    }

    void kill() {
        if (!pid)
            return;
        ::kill(pid, SIGTERM);
        this->wait();
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
        state = "FINISHED";
        pid = 0;
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
        state = "FINISHED";
        pid = 0;
        return true;
    }

    ~process() { this->kill(); }
};
