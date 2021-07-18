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
    using workload_conf_t = typename TRAITS::workload_conf_t;
    using parallel_conf_t = typename TRAITS::parallel_conf_t;
    std::vector<std::shared_ptr<process<parallel_conf_t>>> waiting_processes;
    std::vector<std::shared_ptr<process<parallel_conf_t>>> processes;
    std::map<std::tuple<int, workload_conf_t, parallel_conf_t>, 
             std::vector<std::shared_ptr<process<parallel_conf_t>>>> attempts;
    size_t nattempts;
    size_t nqueued;
    uint64_t oldcs = 0;
    size_t finished = 0, queued = 0, done = 0, error = 0;
    dispatcher(int _nattempts, int _nqueued) : nattempts(_nattempts), nqueued(_nqueued) {}

    void enqueue(int scope_id, workload_conf_t wconf, parallel_conf_t pconf, 
                 std::shared_ptr<input_maker_base<parallel_conf_t>> im,
                 std::shared_ptr<output_maker_base<parallel_conf_t>> om) {
        auto conf = std::make_tuple(scope_id, wconf, pconf);
        auto &already_submitted = attempts[conf];
        std::shared_ptr<process<parallel_conf_t>> proc(std::make_shared<process<parallel_conf_t>>(pconf, im, om));
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
            std::shared_ptr<process<typename TRAITS::parallel_conf_t>> proc = waiting_processes[0];
            execution_environment env;
            auto &im = proc->im;
            im->make(proc->pconf, env);
            if (im->preproc != "")
                proc->preproc = im->preproc;
            if (im->postproc != "")
                proc->postproc = im->postproc;
            if (env.holdover) {
                proc->env = env;
                waiting_processes.erase(waiting_processes.begin());
                waiting_processes.push_back(proc);
                size_t q = 0;
                for (const auto &p : waiting_processes) {
                    if (p->env.holdover) {
                        q++;
                    }
                }
                if (q == waiting_processes.size()) {
                    for (const auto &p : waiting_processes) {
                        auto &e = p->env;
                        e.holdover = false;
                        e.skip = true;
                        p->start(e);
                    }
                    waiting_processes.erase(waiting_processes.begin(), waiting_processes.end());
                    while (!check_if_all_finished()) { ; }
                    break;
                }
                continue;
            }
#if DEBUG  // FIXME consider making this output a command-line switchable option
            std::cout << ">> dispatcher: start: {" << TRAITS::parallel_conf_to_string(proc->pconf) << "} "
                      << env.to_string() << std::endl; 
#endif
            proc->start(env);
            processes.push_back(proc);
            waiting_processes.erase(waiting_processes.begin());
            if (check_if_all_finished())
                break;
            if (queued < 2)
                usleep(1000);
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
                if (procs.size() < nattempts)
                    continue;
                if (procs.size() > nattempts) {
                    assert(0 && "Configuration error: too many processes in a single running group.");
                }
                bool all_finished = true;
                for (auto &proc : procs) {
                    all_finished = all_finished && (proc->state == "FINISHED");
                }
                if (!all_finished)
                    continue;
#ifdef DEBUG // FIXME consider making this output a command-line switchable option
                auto &conf = it.first;
                auto &wconf = std::get<1>(conf);
                auto &pconf = std::get<2>(conf);
                std::cout << ">> dispatcher: " << nattempts << " attempts for: {"
                          << wconf.first << "," << wconf.second << "} in parallel conf: {"
                          << TRAITS::parallel_conf_to_string(pconf)
                          << "} finished, procssing output" << std::endl;
#endif
                procs[0]->om->make(procs);
            }
        }
        return all_is_done;
    }
};
