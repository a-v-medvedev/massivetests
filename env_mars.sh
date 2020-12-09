#!/bin/bash

function env_init_global {
    echo "=== Specific Environment settings for 'mars' host ==="
    script=$(mktemp .XXXXXX.sh)
cat > $script << 'EOM'

#export MPICC=...
#export MPICXX=...
#export CC=...
#export CXX=...

#export CUDA_CC=35
#export CUDA_ARCH="-arch=sm_${CUDA_CC}"
#export CUDA_GENCODE="arch=compute_${CUDA_CC},code=sm_${CUDA_CC}"

export MAKE_PARALLEL_LEVEL=4

export PSUBMIT_OPTS_NNODES=1
export PSUBMIT_OPTS_PPN=4
export PSUBMIT_OPTS_NGPUS=1
export PSUBMIT_OPTS_QUEUE_NAME=test
export PSUBMIT_OPTS_QUEUE_SUFFIX=
export PSUBMIT_OPTS_NODETYPE=
export PSUBMIT_OPTS_INIT_COMMANDS=
export PSUBMIT_OPTS_MPI_SCRIPT=ompi3
export PSUBMIT_OPTS_BATCH_SCRIPT=direct

EOM
    . $script
    cat $script
    rm $script
    echo "============================================================"
}


function env_init {
    local name="$1"
    case "$name" in
    scotch)
        # put here any specific env. setting before scotch build
    ;;
    yaml-cpp)
        # put here any specific env. setting before yaml-cpp build
    ;;
    silo)
        # put here any specific env. setting before silo build
    ;;
    CGNS)
        # put here any specific env. setting before CGNS build
	;;
    qubiq-lib)
        # put here any specific env. setting before qubiq-lib build
    ;;
    qubiq-solver)
        # put here any specific env. setting before qubiq-solver build
    ;;
    esac
    return 0
}

