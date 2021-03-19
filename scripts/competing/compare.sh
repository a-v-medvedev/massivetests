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

FIRST="$1"
SECOND="$2"
PAIRS="$3"

[ ! -d "$FIRST" ] && echo "ERROR: No argument or FIRST directory not exists ($FIRST)" && exit 1
[ ! -d "$SECOND" ] && echo "ERROR: No argument or SECOND directory not exists ($SECOND)" && exit 1
[ ! -f "$FIRST/output.yaml" ] && echo "ERROR: No output.yaml file in FIRST directory ($FIRST)" && exit 1
[ ! -f "$SECOND/output.yaml" ] && echo "ERROR: No output.yaml file in SECOND directory ($FIRST)" && exit 1

for b in $PAIRS; do
    first_bench=$(elem $b 1 :)
    second_bench=$(elem $b 2 :)
    echo $second_bench ":"
    echo "---"
    for n in $MASSIVE_TESTS_NODES; do
        for len in $MASSIVE_TESTS_SIZES; do
            T=$(cat $FIRST/output.yaml | grep $first_bench | grep "{n: $n" | grep "Length: $len," | awk -F '[ :{},]*' '{print int($15*1000000)}')
            OV=$(cat $SECOND/output.yaml | grep $second_bench | grep "{n: $n" | grep "Length: $len," | awk -F '[ :{},]*' '{print int($15*1000000)}')
            [ -z "$T" ] && echo "ERROR: No data for {n=${n},len=${len}} in output.yaml file in FIRST directory ($FIRST)" && exit 1
            [ -z "$OV" ] && echo "ERROR: No data for {n=${n},len=${len}} in output.yaml file in SECOND directory ($SECOND)" && exit 1
            echo $n $len $T $OV
        done
    done
    echo "---"
done
