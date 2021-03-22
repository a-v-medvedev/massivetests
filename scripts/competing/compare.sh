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

function getvalue() {
    local line=$1
    echo "$line" | awk -F '[ :{},]*' '{ for (v=1; v<=NF; v++) { if (flag==1) {flag=0; print $v*1000000} if ($v=="Value") flag=1; } }'
}

function getline() {
    local file=$1
    local keywords="$2"
    local key_block="$3"
	local sizekeyword="$4"
    local n="$5"
    local size="$6"
    local search="/[ {]n: $n,/ && /[ {]${sizekeyword}: ${size},/"
    for i in 1 2 3 4; do
        if [ "$key" != "*" ]; then
            keyword=$(elem $keywords $i :)
            key=$(elem $key_block $i :)
            x="/[ {]${keyword}: ${key},/"
            search="${search} && ${x}"
        fi
    done
	cat "$file" | awk "$search { print }"
}

FIRST="$1"
SECOND="$2"
PAIRS="$3"
KEYWORDS="$4"
SIZEKEYWORD="$5"

[ ! -d "$FIRST" ] && echo "ERROR: No argument or FIRST directory not exists ($FIRST)" && exit 1
[ ! -d "$SECOND" ] && echo "ERROR: No argument or SECOND directory not exists ($SECOND)" && exit 1
[ ! -f "$FIRST/output.yaml" ] && echo "ERROR: No output.yaml file in FIRST directory ($FIRST)" && exit 1
[ ! -f "$SECOND/output.yaml" ] && echo "ERROR: No output.yaml file in SECOND directory ($FIRST)" && exit 1

for b in $PAIRS; do
    first_key_block=$(elem $b 1 ;)
    second_key_block=$(elem $b 2 ;)
    echo $second_key_block ":"
    echo "---"
    for n in $MASSIVE_TESTS_NODES; do
        # FIXME add ppn handling
        for size in $MASSIVE_TESTS_SIZES; do
            V1=$(getvalue $(getline $FIRST/output.yaml "$KEYWORDS" "$first_key_block" "$SIZEKEYWORD" $n $size)) 
            V2=$(getvalue $(getline $SECOND/output.yaml "$KEYWORDS" "$second_key_block" "$SIZEKEYWORD" $n $size)) 
            [ -z "$V1" ] && echo "ERROR: No data for {n=${n},size=${size}} in output.yaml file in FIRST directory ($FIRST)" && exit 1
            [ -z "$V2" ] && echo "ERROR: No data for {n=${n},size=${size}} in output.yaml file in SECOND directory ($SECOND)" && exit 1
            echo $n $size $V1 $V2
        done
    done
    echo "---"
done
