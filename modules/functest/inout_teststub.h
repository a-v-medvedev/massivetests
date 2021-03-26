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

struct input_maker_teststub : public input_maker {
    input_maker_teststub(test_scope<traits> &_scope) : input_maker(_scope) {}
    virtual void make(std::string &input_yaml, std::string &psubmit_options, std::string &args) override {
        input_maker::make(input_yaml, psubmit_options, args);
    }
};

struct output_maker_teststub : public output_maker {
    output_maker_teststub(test_scope<traits> &_scope, const std::string &_outfile) 
        : output_maker(_scope, _outfile) {}
    virtual void make(std::vector<std::shared_ptr<process>> &attempts) override {
        output_maker::make(attempts);
    }
};

} // namespace functest
