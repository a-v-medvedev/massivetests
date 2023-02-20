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
struct input_maker_teststub : public input_maker<parallel_conf_t> {
    input_maker_teststub(test_scope<traits> &_scope) : input_maker<parallel_conf_t>(_scope) {
        input_maker<parallel_conf_t>::result_key = "-output";
    }
    virtual bool make(const parallel_conf_t &pconf, execution_environment &env) override {
        return input_maker<parallel_conf_t>::make(pconf, env);
    }
};

template <typename parallel_conf_t>
struct output_maker_teststub : public output_maker<parallel_conf_t> {
    output_maker_teststub(test_scope<traits> &_scope, const std::string &_outfile) 
        : output_maker<parallel_conf_t>(_scope, _outfile) {}
    virtual void make(std::vector<std::shared_ptr<process<parallel_conf_t>>> &attempts) override {
        output_maker<parallel_conf_t>::make(attempts);
    }
};

} // namespace functest
