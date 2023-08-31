#!/bin/bash
#
#   This file is part of massivetest.
#
#   Massivetest is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    massivetests is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with massivetests.  If not, see <https://www.gnu.org/licenses/>.
#
#

source ./massive_tests.inc

init_env

source ./params.inc
[ -e modeset.inc ] && source ./modeset.inc


MASSIVE_TESTS_WORKLOADS="$WORKLOADS"
MASSIVE_TESTS_CONFS="$CONFS"
MASSIVE_TESTS_SECTIONS="$SECTIONS"
MASSIVE_TESTS_PARAMETERS="$PARAMETERS"
MASSIVE_TESTS_WPRTKEYWORD=${MASSIVE_TESTS_WPRTKEYWORD:-"Workpart"}
MASSIVE_TESTS_KEYWORDS=${MASSIVE_TESTS_KEYWORDS:-"Workload:Conf:X:X"}
TUPLE=$(comb4 "$WORKLOADS" "$CONFS" "$SECTIONS" "$PARAMETERS")
rm -f references.txt
rm -rf summary
rm -rf run
mkdir summary
mkdir run
for mode in $MODES; do
    for submode in $SUBMODES; do
        CONF=conf.${mode}_${submode}
        DIR=run/conf.${mode}_${submode}
        export MASSIVE_TESTS_TESTITEM_MODE=$mode
        export MASSIVE_TESTS_TESTITEM_SUBMODE=$submode
        if is_bash_func_declared set_specific_params; then 
            set_specific_params "$mode" "$submode"
        fi
        massivetest-run
        move_results "$DIR"
        ./extract.sh "$DIR" "$TUPLE" "$MASSIVE_TESTS_KEYWORDS" "$MASSIVE_TESTS_WPRTKEYWORD" > "run/out.$CONF" && true
        if [ "$?" != "0" ]; then
            tail -n1 "run/out.$CONF"
            fatal "./extract.sh failed."
        fi
    done
    NN=$(nelems "$MASSIVE_TESTS_PCONFS")
    NS=$(nelems "$MASSIVE_TESTS_WORKPARTS")
    STEP=$(expr "$NN" \* "$NS" \+ 3)
    ./script-postproc.sh conf.$mode "$STEP" "$TUPLE" || exit 1 && true
done

./make_table.sh "$TUPLE" || exit 1 && true

are_there_files "stats.txt" || exit 1 && true
are_there_files "run/table.*" || exit 1 && true

cp -f stats.txt run/table.* summary/
are_there_files references.txt && cp -f references.txt summary/ || true
are_there_files "test_items*yaml" && cp -f test_items*yaml summary/ || true
are_there_files "input_*.yaml" && cp -f input_*.yaml summary/ || true

