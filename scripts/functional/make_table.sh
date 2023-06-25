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

[ -z "$MASSIVE_TESTS_COLUMN_WIDTH" ] && MASSIVE_TESTS_COLUMN_WIDTH=7

function parse_summary() {
    local summary="$1"
    local table="$2"
    local submodes="$3"
    local nsubmodes="$4"
    cat > .parse.awk <<EOF
        BEGIN {
            S=P=F=N=TACE=0
        }
        NF==4 && \$1!="#" { 
            IDX=\$1 " " \$2
            if (RESULT[IDX] != "")
               RESULT[IDX]=RESULT[IDX] "%"
            RESULT[IDX]=RESULT[IDX] \$3; 
            if (\$4!="-") 
                RESULT[IDX]=RESULT[IDX] "(#" \$4 ")"; 
            if (\$3 == "S") S++;
            else if (\$3 == "P") P++;
            else if (\$3 == "F") F++;
            else if (\$3 == "N") N++;
            else if (\$3 == "T" || \$3 == "A" || \$3 == "C" || \$3 == "E") TACE++;
            else P++;
        } 
        END { 
            for (i in RESULT) { 
                print i " :" RESULT[i] 
            } 
            print S " " P " " F " " N " " TACE >> ".stats.txt"
        }
EOF
              #sub(/@/,"", \$i)
    cat > .format.awk <<EOF
        { 
          str="" 
          for (i=4;i<=NF;i++) {
              item=\$i
              sub(/[ ]*/, "", item)
              if (item=="P") item="PASSD"
              if (item=="S") item="SKIPD"
              if (item=="N") item="NORES"
              fld=sprintf("%-${MASSIVE_TESTS_COLUMN_WIDTH}s",item)
              str=str fld
          }
          printf "%-" WIDTH "s :\n", str;
        }
EOF
    WIDTH=$(expr $(echo $submodes | wc -c))
    WIDTH2=$(expr "$nsubmodes" \* ${MASSIVE_TESTS_COLUMN_WIDTH} + 1)
    [ "$WIDTH2" -gt "$WIDTH" ] && WIDTH=$WIDTH2
    awk -f .parse.awk "$summary" > .table.txt
    cat .table.txt | sort -k1,1n -k2,2n | awk -v WIDTH=$WIDTH -F'[ :%]' -f .format.awk > "$table"
    awk -v "s=$submodes" -v "WIDTH=$WIDTH" 'END { printf("%-" WIDTH "s :\n", s); }' >> "$table" < /dev/null
    rm -f .parse.awk .format.awk .table.txt
}

cat /dev/null > .stats.txt
WORKLOADS="$1"
for wld in $WORKLOADS; do
    made_first_column=""
    n=1
    for src in run/sum.*/out.summary.$wld; do  
        if [ -z "$made_first_column" ]; then
            cat $src | awk 'NF==4 && $1!="#" { printf "%-8s%-24s:\n", $1, $2 }' | sort -k1,1n -k2,2n | uniq > run/table.$wld.0
            echo -e "# nnodes workpart               :" >> run/table.$wld.0
            made_first_column="yes"
        fi
        submodes=$(grep '^#' $src | sed 's/# //')
        nsubmodes=$(echo $submodes | wc -w)
        if [ "${submodes}" == "X" ]; then legend="$src"; else legend="$src (${submodes})"; fi
        sm=$(echo -e "$legend" | sed 's!^run/sum\.conf.!!;s!/out.summary[^ ]*!!')
        parse_summary "$src" "run/table.$wld.$n" "$sm" "$nsubmodes"
        n=$(expr $n \+ 1)
    done
    paste -d ' ' run/table.$wld.* > run/table.$wld
    rm run/table.$wld.*
done
cat .stats.txt | awk '{ for (i=1; i<=NF; i++) SUM[i]+=$i; N=NF; } END { print "S=" SUM[1] " P=" SUM[2] " F=" SUM[3] " N=" SUM[4] " TACE=" SUM[5]; }' > stats.txt
rm .stats.txt


