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

namespace functest {

template <typename parallel_conf_t>
struct input_maker_qubiq : public input_maker<parallel_conf_t> {
    using input_maker<parallel_conf_t>::testitem;
    using input_maker<parallel_conf_t>::scope;
    input_maker_qubiq(test_scope<traits> &_scope) : input_maker<parallel_conf_t>(_scope) {
        input_maker<parallel_conf_t>::load_key = "-yaml"; 
    }
    virtual bool make(const parallel_conf_t &pconf, execution_environment &env) override {
        if (!input_maker<parallel_conf_t>::make(pconf, env)) {
            return false;
        }
        /*
        auto &workload = scope.workload_conf.first;
        
        if (testitem.get_skip_flag(workload, pconf.first, pconf.second)) {
            env.skip = true;
            return;
        }
        */
        auto grid = scope.workparts[0].first;
        assert(grid.size() != 0);
        env.cmdline_args += std::string(" -grid ") + grid;
        env.cmdline_args += std::string(" -remove_decomp yes");
        env.cmdline_args += std::string(" -output_dir out.%PSUBMIT_JOBID%");
        return true;
    }
};

template <typename parallel_conf_t>
struct output_maker_qubiq : public output_maker<parallel_conf_t> {
    output_maker_qubiq(test_scope<traits> &_scope, const std::string &_outfile) 
        : output_maker<parallel_conf_t>(_scope, _outfile) {}
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts) override {
        output_maker<parallel_conf_t>::make(attempts);
    }
};

} // namespace functest
