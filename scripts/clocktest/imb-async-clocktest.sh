#!/bin/bash

source ../massive_tests.inc

[ -f ./env.sh ] && source ./env.sh || fatal "no env.sh file."
env_init_global

set -eu

[ -e psubmit.bin ] || fatal "psubmit.bin directory or symlink required."
export PATH=$PWD/psubmit.bin:$PATH
export LD_LIBRARY_PATH=$PWD/thirdparty/yaml-cpp.bin/lib:$PWD/thirdparty/argsparser.bin:$LD_LIBRARY_PATH
[ -x ./massivetest ] || fatal "massivetest executable is required."

cat /dev/null > local_impi_env.sh
./massivetest --workloads="calc_calibration:tavg" --scale="2:$PSUBMIT_OPTS_PPN" --workparts="4:100" --nqueued="1" --repeats=1 --driver=imb_async 
