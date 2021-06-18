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


BENCHS="$1"
for b in $BENCHS; do
        made_base_out=""
        n=1
        for src in sum.*/out.summary.$b; do  
            if [ -z "$made_base_out" ]; then
                    cat $src | awk 'NF==4 && $1!="#" { print $1 " " $2 " \t:"}' > table.$b.0
                    echo -e "# nnodes len \t:" >> table.$b.0
                    made_base_out=yes
            fi
            submodes=$(grep '^#' $src | sed 's/# //')
            cat $src | awk 'NF==4 && $1!="#" { COMPARE[$1 " " $2]=COMPARE[$1 " " $2] " " $3; if ($4!="-") COMPARE[$1 " " $2]=COMPARE[$1 " " $2] "(#" $4 ")"; } END { for (i in COMPARE) { print i " :" COMPARE[i] } }' | sort -k1,1n -k2,2n | awk '{$1=$2=$3="";gsub(/[ ]*/,""); printf "%-12s:\n", $0;}'> table.$b.$n
            sm=$(echo -e "$src (${submodes})" | sed 's/^sum\.conf.//;s!/out.summary[^ ]*!!')
            awk -v "s=${sm}" 'END { printf("%-12s:\n", s); }' < /dev/null >> table.$b.$n
            n=$(expr $n \+ 1)
        done
        paste -d ' ' table.$b.* > table.$b
        rm table.$b.*
done

