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

namespace functest {

struct input_maker_qubiq : public input_maker {
    input_maker_qubiq(test_scope<traits> &_scope) : input_maker(_scope) {
        load_key = "-yaml"; 
    }
    virtual void make(int n, int ppn, std::string &input_yaml, std::string &psubmit_options, std::string &args) override {
        input_maker::make(n, ppn, input_yaml, psubmit_options, args);
        if (testitem.get_skip_flag(n, ppn)) {
            return;
        }
        auto grid = scope.workload_sizes[0].first;
        assert(grid.size() != 0);
        args += std::string(" -grid ") + grid;
/*        
        args += std::string(" -logfile logfile.%PSUBMIT_JOBID%.log");
        args += std::string(" -test_iters ") + std::to_string(scope.workload_sizes[0].second);
        if (testitem.base.find("solver/iters") != testitem.base.end()) {
            args += std::string(" -solver_params max_iters=") + std::to_string((int)testitem.base["solver/iters"]);
        }
*/
    }
};

struct output_maker_qubiq : public output_maker {
    output_maker_qubiq(test_scope<traits> &_scope, const std::string &_outfile) 
        : output_maker(_scope, _outfile) {}
    virtual void make(std::vector<std::shared_ptr<process>> &attempts) override {
        output_maker::make(attempts);
    }
};

} // namespace functest
