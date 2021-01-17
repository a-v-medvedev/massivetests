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

source ./modules/imb_async/modeset.inc

export MASSIVE_TESTS_DRIVER="imb_async"

# Scope:
export MASSIVE_TESTS_NODES="1 2 4 6 8 12 16"
#export MASSIVE_TESTS_NODES="8 16"
export MASSIVE_TESTS_PPNS=$PSUBMIT_OPTS_PPN

# Sizes:
export MASSIVE_TESTS_SIZES="4 512 16384 131072 4194304"
export MASSIVE_TESTS_REPEATS="1000 1000 500 50 10"
#export MASSIVE_TESTS_SIZES="131072"
#export MASSIVE_TESTS_REPEATS="100"

# Calc cycle duration for each workload size, comma-separated (machine-specific!)
export MASSIVETEST_CALCTIME="10,20,100,2000,25000"

# Workload & params:
WORKLOADS_BASE="sync_pt2pt sync_allreduce sync_na2a sync_rma_pt2pt"
#WORKLOADS_BASE="sync_pt2pt sync_allreduce"
PARAM_BASE="tavg"

WORKLOADS_COMPETING="async_pt2pt async_allreduce async_na2a async_rma_pt2pt"
#WORKLOADS_COMPETING="async_pt2pt async_allreduce"
PARAM_COMPETING="over_full"


# Modes and submodes:
MODES="calc_progress"
#SUBMODES="avg"
SUBMODES="prog1 prog2 prog4 prog6 prog10"
#MODES="calc_mpich_prog_pinned"
#SUBMODES="avg max med"

# Calc cycle calibration parameter (machine-specific!)
export MASSIVETEST_CPER10USEC=$PSUBMIT_OPTS_CPER10USEC

# Number of sumultaineously queued:
export MASSIVE_TESTS_NQUEUED=8
