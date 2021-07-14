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

template <typename parallel_conf_t>
struct input_maker_xamg : public input_maker<parallel_conf_t> {
    using input_maker<parallel_conf_t>::testitem;
    using input_maker<parallel_conf_t>::scope;
    input_maker_xamg(test_scope<traits> &_scope) : input_maker<parallel_conf_t>(_scope) { 
    }
    virtual void make(const parallel_conf_t &pconf, execution_environment &env) override {
        input_maker<parallel_conf_t>::make(pconf, env);
        if (testitem.get_skip_flag(pconf.first, pconf.second)) {
            env.skip = true;
            return;
        }
        auto matrix_name = scope.workparts[0].first;
        assert(matrix_name.size() != 0);
        if (matrix_name[0] == '@') {
            if (matrix_name.size() > 2 && matrix_name[1] == '@') {
                env.cmdline_args += std::string(" -generator_params");
                env.cmdline_args += std::string(" vsz=");
                env.cmdline_args += matrix_name.substr(2);
                env.cmdline_args += std::string(" -matrix ") + scope.workload_conf.first;
            } else {
                auto dim = helpers::str_split(matrix_name.substr(1), 'x');
                env.cmdline_args += std::string(" -matrix generate");
                env.cmdline_args += std::string(" -generator_params"); 
                env.cmdline_args += std::string(" case=cube:");
                env.cmdline_args += std::string("nx=") + dim[0] + ":" + std::string("ny=") + dim[1] + ":" +
                                    std::string("nz=") + dim[2];
            }
        } else {
            env.cmdline_args += std::string(" -matrix ") + matrix_name;
        }
        env.cmdline_args += std::string(" -logfile logfile.%PSUBMIT_JOBID%.log");
        env.cmdline_args += std::string(" -test_iters ") + std::to_string(scope.workparts[0].second);
        if (testitem.base.find("solver/iters") != testitem.base.end()) {
            env.cmdline_args += std::string(" -solver_params max_iters=") + 
                                std::to_string((int)testitem.base["solver/iters"]);
        }
    }
};

template <typename parallel_conf_t>
struct output_maker_xamg : public output_maker<parallel_conf_t> {
    output_maker_xamg(test_scope<traits> &_scope, const std::string &_outfile) 
        : output_maker<parallel_conf_t>(_scope, _outfile) {}
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts) override {
        output_maker<parallel_conf_t>::make(attempts);
    }
};

} // namespace functest
