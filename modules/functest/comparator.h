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

namespace functest {

struct comparator_t {
    std::string parameter_code;
    std::string dir;
    virtual bool acquire_result_data_piece(const YAML::Node &stream, const std::string &section, const std::string &parameter) = 0;
    virtual status_t check_attempts_equality(std::vector<std::shared_ptr<comparator_t>> &v, std::string &comment) = 0;
    virtual status_t compare(std::string &comment) const = 0;
    virtual bool operator<(const comparator_t &other) const = 0;
};

template <typename val_t>
struct basic_value_comparator : public comparator_t {
    val_t result;
    val_t base;
    virtual bool acquire_result_data_piece(const YAML::Node &stream, const std::string &section, const std::string &parameter);
    virtual status_t check_attempts_equality(std::vector<std::shared_ptr<comparator_t>> &v, std::string &comment);
    virtual status_t compare(std::string &comment) const override { (void)comment; return status_t::N; }
    virtual bool operator<(const comparator_t &other) const;
};

template <typename val_t>
struct absolute_numeric_value_comparator : public basic_value_comparator<val_t> {
    using basic_value_comparator<val_t>::parameter_code;
    using basic_value_comparator<val_t>::dir;
    using basic_value_comparator<val_t>::result;
    using basic_value_comparator<val_t>::base;
    val_t tolerance;
    virtual status_t compare(std::string &comment) const override;
};

template <typename val_t>
struct relative_numeric_value_comparator : public basic_value_comparator<val_t> {
    using basic_value_comparator<val_t>::parameter_code;
    using basic_value_comparator<val_t>::dir;
    using basic_value_comparator<val_t>::result;
    using basic_value_comparator<val_t>::base;
    val_t tolerance;
    virtual status_t compare(std::string &comment) const override;
};

template <typename val_t>
struct absolute_nonnumeric_value_comparator : public basic_value_comparator<val_t> {
    using basic_value_comparator<val_t>::parameter_code;
    using basic_value_comparator<val_t>::dir;
    using basic_value_comparator<val_t>::result;
    using basic_value_comparator<val_t>::base;
    virtual status_t compare(std::string &comment) const override;
};

}
