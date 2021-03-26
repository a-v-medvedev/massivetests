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

export MASSIVE_TESTS_DRIVER="imb_async"
export MASSIVE_TESTS_APPLICATION="imb_async"

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
export MASSIVE_TESTS_SIZES="4 512 16384 131072 4194304"
export MASSIVE_TESTS_REPEATS="1000 1000 500 50 10"


# Workloads, sections and params:
WORKLOADS_BASE="X"
CONFS_BASE="X"
SECTIONS_BASE="sync_pt2pt sync_allreduce sync_na2a" # sync_rma_pt2pt"
PARAMETERS_BASE="tavg"

WORKLOADS_COMPETING="X"
CONFS_COMPETING="X"
SECTIONS_COMPETING="async_pt2pt async_allreduce async_na2a" # async_rma_pt2pt"
PARAMETERS_COMPETING="over_full"


# Modes and submodes:
MODES="none calc calc_progress"
SUBMODES="avg"


# Output file structure:
export MASSIVE_TESTS_KEYWORDS="X:X:Benchmark:Parameter"
export MASSIVE_TESTS_SIZEKEYWORD="Length"

function get_value() {
    local line=$1
    echo "$line" | awk -F '[ :{},]*' '{ for (v=1; v<=NF; v++) { if (flag==1) {flag=0; print $v*1000000} if ($v=="Value") flag=1; } }'
}

#-----------------------------------------------
# Additional (not handled in script):
# --- Calc cycle duration for each workload size, comma-separated (machine-specific!)
export MASSIVETEST_CALCTIME="10,20,100,2000,25000"

# --- Calc cycle calibration parameter (machine-specific!)
export MASSIVETEST_CPER10USEC=$PSUBMIT_OPTS_CPER10USEC
[ -z "$MASSIVETEST_CPER10USEC" ] && fatal "PSUBMIT_OPTS_CPER10USEC is not set."
