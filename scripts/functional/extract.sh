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
source ./params.inc

check_bash_func_declared get_value


function getline() {
    local file=$1
    local keywords="$2"
    local key_block="$3"
	local sizekeyword="$4"
    local n="$5"
    local size="$6"
    local search="/[ {]n: $n,/ && /[ {]${sizekeyword}: [\"]*${size}[\"]*,/"
    for i in 1 2 3 4; do
        keyword=$(elem "$keywords" "$i" :)
        if [ "$keyword" != "X" ]; then
            key=$(elem "$key_block" "$i" :)
            x="/[ {};]${keyword}: [\"]*${key}[\"]*,/"
            search="${search} && ${x}"
        fi
    done
	cat "$file" | awk "$search { print }"
}

function get_dir() {
    local line="$1"
    echo "$line" | awk -F'[ =;")}]' '{ for (i=1;i<=NF;i++) { if ($i=="dir") flag=1; else if (flag==1) { print $i; break; } } }'
}

function get_ref_number() {
    local dir="$1"
    local value="$2"
    local line="$3"
    if [ "$dir" == "-" ]; then
        echo "-"
    else
        local STATUS=""
        case "$value" in
            F) STATUS="FAILED"
                ;;
            N) STATUS="NONZERO"
                ;;
            T) STATUS="TIMEOUT"
                ;;
            A) STATUS="ASSERT"
                ;;
            C) STATUS="CRASH"
                ;;
            E) STATUS="EXCEPTION"
                ;;
        esac
        [ -e "references.txt" ] || touch references.txt
        N=$(wc -l < references.txt)
        NN=$(expr "$N" \+ 1)
        name=$(basename $dir)
        conf=$(dirname $dir)
        echo "${NN}) ${STATUS}: ${name}/  ---  $conf  ---  $line" >> references.txt
        echo $NN
        cp -a $dir summary/
    fi
}

DIR="$1"
PARAMS="$2"
KEYWORDS="$3"
SIZEKEYWORD="$4"

[ ! -d "$DIR" ] && echo "ERROR: No argument or directory for extraction does not exist (dir=$DIR)" && exit 1
[ ! -f "$DIR/output.yaml" ] && echo "ERROR: No output.yaml file in the directory for extraction (dir=$DIR))" && exit 1

for p in $PARAMS; do
    echo $p ":"
    echo "---"
    for n in $MASSIVE_TESTS_NODES; do
        # FIXME add ppn handling
        for size in $MASSIVE_TESTS_WORKPARTS; do
            L=$(getline $DIR/output.yaml "$KEYWORDS" "$p" "$SIZEKEYWORD" "$n" "$size")
            V=$(get_value "$L") 
            D=$(get_dir "$L")
            [ -z "$D" ] && D=-
            [ "$D" != "-" ] && D=$DIR/$D
            [ -z "$V" ] && echo "ERROR: No data for {n=${n},size=${size}} in output.yaml file in the directory for extraction ($DIR)" && exit 1
            echo $n $size $V $(get_ref_number "$D" "$V" "$L")
        done
    done
    echo "---"
done
