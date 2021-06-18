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

source ./massive_tests.inc

init_env

source ./params.inc
source ./modeset.inc

check_bash_func_declared set_specific_params

MASSIVE_TESTS_WORKLOADS="$WORKLOADS"
MASSIVE_TESTS_CONFS="$CONFS"
MASSIVE_TESTS_SECTIONS="$SECTIONS"
MASSIVE_TESTS_PARAMETERS="$PARAMETERS"
TUPLE=$(comb4 "$WORKLOADS" "$CONFS" "$SECTIONS" "$PARAMETERS")
rm -f references.txt
rm -rf summary
mkdir summary
for mode in $MODES; do
    for submode in $SUBMODES; do
        CONF=conf.${mode}_${submode}
        set_specific_params "$mode" "$submode"
        massivetest-run
        move_results "$CONF"
        ./extract.sh "$CONF" "$TUPLE" "$MASSIVE_TESTS_KEYWORDS" "$MASSIVE_TESTS_SIZEKEYWORD" > "out.$CONF" && true
        if [ "$?" != "0" ]; then
            tail -n1 "out.$CONF"
            fatal "./extract.sh failed."
        fi
    done
    NN=$(nelems "$MASSIVE_TESTS_NODES")
    NS=$(nelems "$MASSIVE_TESTS_SIZES")
    STEP=$(expr "$NN" \* "$NS" \+ 3)
    ./script-postproc.sh conf.$mode "$STEP" "$TUPLE" || exit 1 && true
done

./make_table.sh "$TUPLE" || exit 1 && true

touch references.txt
cp references.txt table.* test_items.yaml input_*.yaml summary/ || exit 1 && true

