#!/bin/bash
#
#   This file is part of massivetest.
#
#   Massivetest is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Foobar is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
#
#

function fatal() {
    local str="$1"
    echo "FATAL: $str"
    exit 1
}

function warning() {
    local str="$1"
    echo "WARNING: $str"
}

function info() {
    local str="$1"
    echo "$str"
}

function check_bash_func_declared() {
    [ `type -t $1`"" == 'function' ] || fatal "$1 bash function is not defined!"
}

function zip1() {
    local result=""
    for i in $1; do
        result="${result},${i}:$2"
    done
    echo $result | sed 's/^,//'
}

function nelems() {
    echo $1 | awk '{print NF}'
}

function elem() {
    local n="$2"
    set +u
    local delim=$3
    set -u
    [ -z "$delim" ] && delim=" "
    echo $1 | awk -F "$3" "{print \$$n}"
}

function zip() {
    local result=""
    local n=0
    set +u
    local delim="$3"
    set -u
    [ -z "$delim" ] && delim=","
    local ne=$(nelems "$2")
    if [ "$ne" == 1 ]; then
        zip1 "$1" "$2"
        return
    fi
    for i in $1; do
        n=$(expr $n \+ 1)
        j=$(echo $2 | awk "{print \$$n}")
        result="${result}${delim}${i}:$j"
    done
    echo $result | sed "s/^${delim}//"
}

function move_results() {
    local DIR="$1"
    rm -rf "$DIR"
    mkdir "$DIR"
    mv results.* "$DIR"
    mv input*.yaml "$DIR"
    mv output_initial.yaml "$DIR"/output.yaml
}

function massivetest-run() {
    local NPPNS=$(zip "$MASSIVE_TESTS_NODES" "$MASSIVE_TESTS_PPNS")
    local SIZES=$(zip "$MASSIVE_TESTS_SIZES" "$MASSIVE_TESTS_REPEATS")
    local WORKLOADS=$(zip "$MASSIVE_TESTS_WORKLOADS" "$MASSIVE_TESTS_PARAMS")
    ./massivetest --driver="$MASSIVE_TESTS_DRIVER" --scale=${NPPNS} --nqueued=$MASSIVE_TESTS_NQUEUED --sizes=$SIZES --repeats=$MASSIVE_TESTS_EXEC_REPEATS --workloads=$WORKLOADS
}

