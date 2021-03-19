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


MODE="$1"
STEP="$2"
BENCHS="$3"

rm -rf sum.$MODE
mkdir sum.$MODE

i=0
for b in $BENCHS; do
    i=$(expr $i \+ 1)
    rm -f sum.${MODE}/out.summary.$b
    SUBMODES=""
    for f in out.${MODE}_*; do
        SUBMODE=$(echo $f | sed "s/out.${MODE}_//")
        echo $SUBMODE | grep -q "_" && continue
        SUBMODES="$SUBMODES $SUBMODE"
        echo ${MODE}_${SUBMODE} >> sum.${MODE}/out.summary.$b
        echo "-----------" >> sum.${MODE}/out.summary.$b
        head -n$(expr $STEP \* $i) $f | tail -n$STEP >> sum.${MODE}/out.summary.$b
    done
    echo "#$SUBMODES" >> sum.${MODE}/out.summary.$b
done
