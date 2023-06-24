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

function is_bash_func_declared() {
    [ `type -t $1`"" == 'function' ]
}

function check_bash_func_declared() {
    is_bash_func_declared  "$1" || fatal "$1 bash function is not defined!"
}

function are_there_files() {
    [ $(ls -1d $1 2>/dev/null | wc -l) != "0" ]
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

function comb() {
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
    for i in $1; do
        for j in $2; do
            result="${result}${delim}${i}${delim_local}$j"
        done
    done
    echo $result | sed "s/^${delim}//"
}

function comb4() {
    local a1="$1"
    local a2="$2"
    local a3="$3"
    local a4="$4"
    local delim="$5"
    local delim_local="$6"
    [ -z "$delim" ] && delim=" "
    [ -z "$delim_local" ] && delim_local=":"
    local Z1=$(zip "$a3" "$a4" "$delim" "$delim_local")
    local Z2=$(comb "$a2" "$Z1" "$delim" "$delim_local")
    local Z3=$(comb "$a1" "$Z2" "$delim" "$delim_local")
    echo "$Z3"
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
    [ $(ls -1d input*.yaml 2>/dev/null | wc -l) == "0" ] || cp input*.yaml "$DIR"
    mv output_initial.yaml "$DIR"/output.yaml
}

function massivetest-run() {
    local NPPNS=$(zip "$MASSIVE_TESTS_NODES" "$MASSIVE_TESTS_PPNS" ",")
    local WORKPARTS=$(zip "$MASSIVE_TESTS_WORKPARTS" "$MASSIVE_TESTS_REPEATS" ",")
    local WORKLOADS=$(comb "$MASSIVE_TESTS_WORKLOADS" "$MASSIVE_TESTS_CONFS" ",")
    local PARAMS=$(comb "$MASSIVE_TESTS_SECTIONS" "$MASSIVE_TESTS_PARAMETERS" ",")
    ./massivetest --driver="$MASSIVE_TESTS_DRIVER" --scale="${NPPNS}" --nqueued="$MASSIVE_TESTS_NQUEUED" --workparts="$WORKPARTS" --repeats="$MASSIVE_TESTS_EXEC_REPEATS" --workloads="$WORKLOADS" --parameters="$PARAMS"
}

function init_env()  {
    [ -f ./env.sh ] && source ./env.sh || fatal "no env.sh file."
    env_init_global

    [ -e psubmit.bin ] || fatal "psubmit.bin directory or symlink required."
    [ -e thirdparty ] || fatal "thirdparty directory or symlink required."
    export PATH=$PWD/psubmit.bin:$PATH
    export LD_LIBRARY_PATH=$PWD/lib:$PWD/thirdparty/yaml-cpp.bin/lib:$PWD/thirdparty/argsparser.bin:$LD_LIBRARY_PATH
    [ -x ./massivetest ] || fatal "massivetest executable is required."
    [ -e ./params.inc ] || fatal "params.inc executable is required."
}

function get_line_from_output_yaml() {
    local file=$1
    local keywords="$2"
    local key_block="$3"
    local wrptkeyword="$4"
    local wrpt="$5"
    local n="$6"
    local search="/[ {]n: $n,/ && /[ {]${wrptkeyword}: [\"]*${wrpt}[\"]*,/"
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

function get_value_simple() {
    local line=$1
    echo "$line" | awk -F '[ :{},]*' '{ for (v=1; v<=NF; v++) { if (flag==1) {flag=0; print $v} if ($v=="Value") flag=1; } }'
}

function parse_yaml {
   local prefix=$2
   local s='[[:space:]]*' w='[a-zA-Z0-9_]*' fs=$(echo @|tr @ '\034')
   echo "$1" | sed -ne "s|,$s\]$s\$|]|" \
        -e ":1;s|^\($s\)\($w\)$s:$s\[$s\(.*\)$s,$s\(.*\)$s\]|\1\2: [\3]\n\1  - \4|;t1" \
        -e "s|^\($s\)\($w\)$s:$s\[$s\(.*\)$s\]|\1\2:\n\1  - \3|;p" | \
   sed -ne "s|,$s}$s\$|}|" \
        -e ":1;s|^\($s\)-$s{$s\(.*\)$s,$s\($w\)$s:$s\(.*\)$s}|\1- {\2}\n\1  \3: \4|;t1" \
        -e    "s|^\($s\)-$s{$s\(.*\)$s}|\1-\n\1  \2|;p" | \
   sed -ne "s|^\($s\):|\1|" \
        -e "s|^\($s\)-$s[\"']\(.*\)[\"']$s\$|\1$fs$fs\2|p" \
        -e "s|^\($s\)-$s\(.*\)$s\$|\1$fs$fs\2|p" \
        -e "s|^\($s\)\($w\)$s:$s[\"']\(.*\)[\"']$s\$|\1$fs\2$fs\3|p" \
        -e "s|^\($s\)\($w\)$s:$s\(.*\)$s\$|\1$fs\2$fs\3|p" | \
   awk -F$fs '{
      indent = length($1)/2;
      vname[indent] = $2;
      for (i in vname) {if (i > indent) {delete vname[i]; idx[i]=0}}
      if(length($2)== 0){  vname[indent]= ++idx[indent] };
      if (length($3) > 0) {
         vn=""; for (i=0; i<indent; i++) { vn=(vn)(vname[i])("_")}
         printf("%s%s%s=\"%s\"\n", "'$prefix'",vn, vname[indent], $3);
      }
   }'
}

function get_value_aux() {
    local line="$1"
    eval $(parse_yaml "$line" "local")
    if [ ! -z "$local1_Auxvalues" ]; then
        local aux=$(echo $line | sed 's/^.*Auxvalues: /- /;s/}$//')
        eval $(parse_yaml "$aux" "aux")
    fi
}

