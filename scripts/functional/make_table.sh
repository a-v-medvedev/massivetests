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
            if (\$3 == "P") P++;
            if (\$3 == "F") F++;
            if (\$3 == "N") N++;
            if (\$3 == "T" || \$3 == "A" || \$3 == "C" || \$3 == "E") TACE++;
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
              fld=sprintf("%-7s",item)
              str=str fld
          }
          printf "%-" WIDTH "s :\n", str;
        }
EOF
    WIDTH=$(expr $(echo $submodes | wc -c))
    WIDTH2=$(expr "$nsubmodes" \* 7 + 1)
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
    for src in sum.*/out.summary.$wld; do  
        if [ -z "$made_first_column" ]; then
            cat $src | awk 'NF==4 && $1!="#" { printf "%-8s%-24s:\n", $1, $2 }' | sort -k1,1n -k2,2n | uniq > table.$wld.0
            echo -e "# nnodes workpart               :" >> table.$wld.0
            made_first_column="yes"
        fi
        submodes=$(grep '^#' $src | sed 's/# //')
        nsubmodes=$(echo $submodes | wc -w)
        sm=$(echo -e "$src (${submodes})" | sed 's/^sum\.conf.//;s!/out.summary[^ ]*!!')
        parse_summary "$src" "table.$wld.$n" "$sm" "$nsubmodes"
        n=$(expr $n \+ 1)
    done
    paste -d ' ' table.$wld.* > table.$wld
    rm table.$wld.*
done
cat .stats.txt | awk '{ for (i=1; i<=NF; i++) SUM[i]+=$i; N=NF; } END { print "S=" SUM[1] " P=" SUM[2] " F=" SUM[3] " N=" SUM[4] " TACE=" SUM[5]; }' > stats.txt
rm .stats.txt


