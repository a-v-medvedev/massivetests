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


source massive_tests.inc

[ -f ./env.sh ] && source ./env.sh || fatal "no env.sh file."
env_init_global

set -eu

[ -e psubmit.bin ] || fatal "psubmit.bin directory or symlink required."
export PATH=$PWD/psubmit.bin:$PATH
export LD_LIBRARY_PATH=$PWD/thirdparty/yaml-cpp.bin/lib:$PWD/thirdparty/argsparser.bin:$LD_LIBRARY_PATH
[ -x ./massivetest ] || fatal "massivetest executable is required."

source modules/imb_async/params.inc

check_bash_func_declared set_specific_params

if [ ! -d base ]; then
    MASSIVE_TESTS_WORKLOADS="$WORKLOADS_BASE"
    MASSIVE_TESTS_PARAMS="$PARAM_BASE"
    MASSIVE_TESTS_EXEC_REPEATS=5
    set_specific_params "base" "base"
    massivetest-run
    move_results base
fi

export MASSIVE_TESTS_WORKLOADS="$WORKLOADS_COMPETING"
MASSIVE_TESTS_PARAMS="$PARAM_COMPETING"
MASSIVE_TESTS_EXEC_REPEATS=5
PAIRS_TO_COMPARE=$(zip "$WORKLOADS_BASE" "$WORKLOADS_COMPETING" " ")
for mode in $MODES; do
    for submode in $SUBMODES; do
        COMPETING=competing.${mode}_${submode}
        set_specific_params "$mode" "$submode"
        massivetest-run
        move_results "$COMPETING"
        ./compare.sh "base" "$COMPETING" "$PAIRS_TO_COMPARE" > "out.$COMPETING" && true
        if [ "$?" != "0" ]; then
            tail -n1 "out.$COMPETING"
            fatal "./compare.sh failed."
        fi
    done
    NN=$(nelems "$MASSIVE_TESTS_NODES")
    NS=$(nelems "$MASSIVE_TESTS_SIZES")
    STEP=$(expr "$NN" \* "$NS" \+ 3)
    ./script-postproc.sh competing.$mode "$STEP" "$MASSIVE_TESTS_WORKLOADS"
done

./make_table.sh "$MASSIVE_TESTS_WORKLOADS"

