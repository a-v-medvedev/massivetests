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
    MASSIVE_TESTS_SECTIONS="$SECTIONS_BASE"
    MASSIVE_TESTS_PARAMETERS="$PARAMETERS_BASE"
    MASSIVE_TESTS_EXEC_REPEATS=$MASSIVE_TESTS_EXEC_REPEATS_BASE
    set_specific_params "base" "base"
    massivetest-run
    move_results "base"
fi

KEYWORDS="*:*:Benchmark:Parameter"
SIZEKEYWORD="Length"

MASSIVE_TESTS_SECTIONS="$SECTIONS_COMPETING"
MASSIVE_TESTS_PARAMETERS="$PARAMETERS_COMPETING"
MASSIVE_TESTS_EXEC_REPEATS=$MASSIVE_TESTS_EXEC_REPEATS_COMPETING
PAIRS_TO_COMPARE=$(zip "$SECTIONS_BASE" "$SECTIONS_COMPETING" " " ";")
for mode in $MODES; do
    for submode in $SUBMODES; do
        COMPETING=competing.${mode}_${submode}
        set_specific_params "$mode" "$submode"
        massivetest-run
        move_results "$COMPETING"
        ./compare.sh "base" "$COMPETING" "$PAIRS_TO_COMPARE" "$KEYWORDS" "$SIZEKEYWORD" > "out.$COMPETING" && true
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

