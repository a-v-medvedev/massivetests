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
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <memory>

#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include <cmath>

template <typename TRAITS>
class test_scope;

template <typename TRAITS>
struct dispatcher {
    const std::vector<std::string> possible_states{"NONE", "Q", "E", "R", "DONE", "FINISHED"};
    using parallel_conf_t = typename TRAITS::parallel_conf_t;
    std::vector<std::shared_ptr<process>> waiting_processes;
    std::vector<std::shared_ptr<process>> processes;
    std::map<parallel_conf_t, std::vector<std::shared_ptr<process>>> attempts;
    size_t nattempts;
    size_t nqueued;
    uint64_t oldcs = 0;
    size_t finished = 0, queued = 0, done = 0, error = 0;
    dispatcher(int _nattempts, int _nqueued) : nattempts(_nattempts), nqueued(_nqueued) {}

    void enqueue(parallel_conf_t conf, std::shared_ptr<input_maker_base> im,
                 std::shared_ptr<output_maker_base> om) {
        auto &already_submitted = attempts[conf];
        std::shared_ptr<process> proc(std::make_shared<process>(conf, im, om));
        already_submitted.push_back(proc);
        waiting_processes.push_back(proc);
        start_more_processes();
    }

    void start_more_processes() {
        static bool inside = false;
        if (inside)
            return;
        inside = true;
        while (
            (processes.size() - finished < nqueued) ||
            ((finished || done) && (queued < 2) && (processes.size() - finished < 2 * nqueued))) {
            if (waiting_processes.size() == 0)
                break;
            std::shared_ptr<process> proc = waiting_processes[0];
            std::string input_yaml, psubmit_options, args;
            proc->im->make(input_yaml, psubmit_options, args);
#if DEBUG
            std::cout << ">> dispatcher: start: {" << proc->n << "," << proc->ppn
                      << "} input=" << input_yaml << " run.options=" << psubmit_options << " args={"
                      << args << "} }" << std::endl;
#endif
            proc->start(input_yaml, psubmit_options, args);
            processes.push_back(proc);
            waiting_processes.erase(waiting_processes.begin());
            if (check_if_all_finished())
                break;
            if (queued < 2)
                usleep(10000);
        }
        inside = false;
    }

    void print_state(std::map<std::string, size_t> &proc_states) {
        uint64_t cs = 0;
        size_t shift = 0;
        for (const auto &s : possible_states) {
            cs ^= (proc_states[s] << shift);
            shift += 8;
        }
        if (cs != oldcs) {
            for (const auto &s : possible_states) {
                std::cout << s << ": " << proc_states[s];
                if (s != possible_states.back())
                    std::cout << ", ";
            }
            std::cout << std::endl;
            oldcs = cs;
        }
    }

    bool check_if_all_finished() {
        assert(nattempts > 0);
        start_more_processes();
        size_t cnt = 0;
        std::map<std::string, size_t> proc_states;
        for (auto &proc : processes) {
            bool res = proc->update_state();
            proc_states[proc->state]++;
            if (res)
                cnt++;
        }
        bool some_new_finished_processes = false;
        if (finished != proc_states["FINISHED"]) {
            some_new_finished_processes = true;
            finished = proc_states["FINISHED"];
        }
        if (some_new_finished_processes)
            print_state(proc_states);
        queued = proc_states["Q"];
        done = proc_states["DONE"];
        error = proc_states["E"];
        assert(finished == cnt);
        bool all_is_done = ((cnt == processes.size()) && (waiting_processes.size() == 0));
        if (some_new_finished_processes || all_is_done) {
            for (auto &it : attempts) {
                auto &procs = it.second;
                if (procs.size() != nattempts)
                    continue;
                bool all_finished = true;
                for (auto &proc : procs) {
                    all_finished = all_finished && (proc->state == "FINISHED");
                }
                if (!all_finished)
                    continue;
#ifdef DEBUG
                std::cout << ">> dispatcher: " << nattempts << " attempts for: {"
                          << (it.first).first << "," << (it.first).second
                          << "} finished, procssing output" << std::endl;
#endif
                procs[0]->om->make(procs);
            }
        }
        return all_is_done;
    }
};
