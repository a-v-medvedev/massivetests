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

function zip_X1() {
    local result=""
    local delim="$3"
    local delim_local="$4"
    for i in $1; do
        result="${result}${delim}${i}${delim_local}$2"
    done
    echo $result | sed "s/^${delim}//"
}

function zip_1X() {
    local result=""
    local delim="$3"
    local delim_local="$4"
    for i in $2; do
        result="${result}${delim}$1${delim_local}$i"
    done
    echo $result | sed "s/^${delim}//"
}

function nelems() {
    echo $1 | awk '{print NF}'
}

function elem() {
    local n="$2"
    set +u
    local delim="$3"
    set -u
    [ -z "$delim" ] && delim=" "
    echo $1 | awk -F "$delim" "{print \$$n}"
}

function zip() {
    local result=""
    local n=0
    set +u
    local delim="$3"
    local delim_local="$4"
    set -u
    [ -z "$delim" ] && delim=" "
    [ -z "$delim_local" ] && delim_local=":"
    local ne_right=$(nelems "$2")
    if [ "$ne_right" == 1 ]; then
        zip_X1 "$1" "$2" "$delim" "$delim_local"
        return
    fi
    local ne_left=$(nelems "$1")
    if [ "$ne_left" == 1 ]; then
        zip_1X "$1" "$2" "$delim" "$delim_local"
        return
    fi
    [ "$ne_right" == "$ne_left" ] || fatal "zip: lists of different size in args ($ne_right and $ne_left)."
    for i in $1; do
        n=$(expr "$n" \+ 1)
        j=$(echo $2 | awk "{print \$$n}")
        result="${result}${delim}${i}${delim_local}$j"
    done
    echo $result | sed "s/^${delim}//"
}

function zip4() {
    local a1="$1"
    local a2="$2"
    local a3="$3"
    local a4="$4"
    local delim="$5"
    local delim_local="$6"
    [ -z "$delim" ] && delim=" "
    [ -z "$delim_local" ] && delim_local=":"
    local Z1=$(zip "$a3" "$a4" "$delim" "$delim_local")
    local Z2=$(zip "$a2" "$Z1" "$delim" "$delim_local")
    local Z3=$(zip "$a1" "$Z2" "$delim" "$delim_local")
    echo "$Z3"
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
    local NPPNS=$(zip "$MASSIVE_TESTS_NODES" "$MASSIVE_TESTS_PPNS" ",")
    local SIZES=$(zip "$MASSIVE_TESTS_SIZES" "$MASSIVE_TESTS_REPEATS" ",")
    local WORKLOADS=$(zip "$MASSIVE_TESTS_WORKLOADS" "$MASSIVE_TESTS_MODES" ",")
    local PARAMS=$(zip "$MASSIVE_TESTS_SECTIONS" "$MASSIVE_TESTS_PARAMETERS" ",")
    ./massivetest --driver="$MASSIVE_TESTS_DRIVER" --scale="${NPPNS}" --nqueued="$MASSIVE_TESTS_NQUEUED" --sizes="$SIZES" --repeats="$MASSIVE_TESTS_EXEC_REPEATS" --workloads="$WORKLOADS" --parameters="$PARAMS"
}

function init_env()  {
    [ -f ./env.sh ] && source ./env.sh || fatal "no env.sh file."
    env_init_global

    [ -e psubmit.bin ] || fatal "psubmit.bin directory or symlink required."
    [ -e thirdparty ] || fatal "thirdparty directory or symlink required."
    export PATH=$PWD/psubmit.bin:$PATH
    export LD_LIBRARY_PATH=$PWD/thirdparty/yaml-cpp.bin/lib:$PWD/thirdparty/argsparser.bin:$LD_LIBRARY_PATH
    [ -x ./massivetest ] || fatal "massivetest executable is required."
    [ -e ./params.inc ] || fatal "params.inc executable is required."
    [ -e ./modeset.inc ] || fatal "modeset.inc executable is required."
}
