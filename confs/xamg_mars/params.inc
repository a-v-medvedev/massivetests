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

export MASSIVE_TESTS_DRIVER="functest"


# Number of sumultaineously queued:
export MASSIVE_TESTS_NQUEUED=1


# Repeats in execution:
MASSIVE_TESTS_EXEC_REPEATS_BASE=1
MASSIVE_TESTS_EXEC_REPEATS_COMPETING=3


# Scope:
export MASSIVE_TESTS_NODES="2 4"
export MASSIVE_TESTS_PPNS=1
[ -z "$MASSIVE_TESTS_PPNS" ] && fatal "PSUBMIT_OPTS_PPN is not set."


# Sizes:
export MASSIVE_TESTS_SIZES="@10x10x10"
export MASSIVE_TESTS_REPEATS="3"


# Workloads, sections and params:
WORKLOADS_BASE="pjacobi"
CONFS_BASE="NV1 NV2 NV4 NV8"
SECTIONS_BASE="X" 
PARAMETERS_BASE="X"

WORKLOADS_COMPETING="pjacobi"
CONFS_COMPETING="NV1 NV2 NV4 NV8"
SECTIONS_COMPETING="X" 
PARAMETERS_COMPETING="X"


# Modes and submodes:
MODES="1:1 2:1 4:1 1:4 2:2"
SUBMODES="spmv solve"


# Output file structure:
export MASSIVE_TESTS_KEYWORDS="Workload:Mode:X:X"
export MASSIVE_TESTS_SIZEKEYWORD="Matrix"

function get_value() {
    local line=$1
    echo "$line" | awk -F '[ :{},]*' '{ for (v=1; v<=NF; v++) { if (flag==1) {flag=0; print $v} if ($v=="Value") flag=1; } }'
}
