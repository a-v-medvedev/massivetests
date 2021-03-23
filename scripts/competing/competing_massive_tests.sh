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

source ../massive_tests.inc

init_env

source ./params.inc
source ./modeset.inc

check_bash_func_declared set_specific_params

if [ ! -d base ]; then
    MASSIVE_TESTS_WORKLOADS="$WORKLOADS_BASE"
    MASSIVE_TESTS_MODES="$MODES_BASE"
    MASSIVE_TESTS_SECTIONS="$SECTIONS_BASE"
    MASSIVE_TESTS_PARAMETERS="$PARAMETERS_BASE"
    MASSIVE_TESTS_EXEC_REPEATS=$MASSIVE_TESTS_EXEC_REPEATS_BASE
    set_specific_params "base" "base"
    massivetest-run
    move_results "base"
fi

#MASSIVE_TESTS_KEYWORDS="*:*:Benchmark:Parameter"
#MASSIVE_TESTS_SIZEKEYWORD="Length"
MASSIVE_TESTS_WORKLOADS="$WORKLOADS_COMPETING"
MASSIVE_TESTS_MODES="$MODES_COMPETING"
MASSIVE_TESTS_SECTIONS="$SECTIONS_COMPETING"
MASSIVE_TESTS_PARAMETERS="$PARAMETERS_COMPETING"
MASSIVE_TESTS_EXEC_REPEATS=$MASSIVE_TESTS_EXEC_REPEATS_COMPETING
TUPLE_BASE=$(zip4 "$WORKLOADS_BASE" "$MODES_BASE" "$SECTIONS_BASE" "$PARAMETERS_BASE")
TUPLE_COMPETING=$(zip4 "$WORKLOADS_COMPETING" "$MODES_COMPETING" "$SECTIONS_COMPETING" "$PARAMETERS_COMPETING")
PAIRS_TO_COMPARE=$(zip "$TUPLE_BASE" "$TUPLE_COMPETING" " " ";")
for mode in $MODES; do
    for submode in $SUBMODES; do
        COMPETING=competing.${mode}_${submode}
        set_specific_params "$mode" "$submode"
        massivetest-run
        move_results "$COMPETING"
        ./compare.sh "base" "$COMPETING" "$PAIRS_TO_COMPARE" "$MASSIVE_TESTS_KEYWORDS" "$MASSIVE_TESTS_SIZEKEYWORD" > "out.$COMPETING" && true
        if [ "$?" != "0" ]; then
            tail -n1 "out.$COMPETING"
            fatal "./compare.sh failed."
        fi
    done
    NN=$(nelems "$MASSIVE_TESTS_NODES")
    NS=$(nelems "$MASSIVE_TESTS_SIZES")
    STEP=$(expr "$NN" \* "$NS" \+ 3)
    ./script-postproc.sh competing.$mode "$STEP" "$MASSIVE_TESTS_SECTIONS"
done

./make_table.sh "$MASSIVE_TESTS_SECTIONS"

