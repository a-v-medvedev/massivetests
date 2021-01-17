function env_init_global {
    echo "=== Specific Environment settings for Lomonosov-2 supercomputer ==="
    script=$(mktemp .XXXXXX.sh)
cat > $script << 'EOM'
module load slurm gcc/9.1 cmake/3.12.3 mkl/2017.1 openmpi/4.0.1-icc2019

export DNB_NOCUDA=1

#export CUDA_CC=35
#export CUDA_ARCH="-arch=sm_${CUDA_CC}"
#export CUDA_GENCODE="arch=compute_${CUDA_CC},code=sm_${CUDA_CC}"

export MAKE_PARALLEL_LEVEL=8

export PSUBMIT_OPTS_NNODES=4
export PSUBMIT_OPTS_PPN=12
export PSUBMIT_OPTS_NGPUS=1
export PSUBMIT_OPTS_QUEUE_NAME=test
export PSUBMIT_OPTS_QUEUE_SUFFIX=
export PSUBMIT_OPTS_NODETYPE=
export PSUBMIT_OPTS_INIT_COMMANDS=
export PSUBMIT_OPTS_INJOB_INIT_COMMANDS='export PATH=/opt/mpi/openmpi-4.0.1-icc2019/bin:$PATH; export LD_LIBRARY_PATH=/opt/mpi/openmpi-4.0.1-icc2019/lib:$LD_LIBRARY_PATH'
export PSUBMIT_OPTS_MPI_SCRIPT=ompi4
export PSUBMIT_OPTS_BATCH_SCRIPT=vbbs
export PSUBMIT_OPTS_CPER10USEC=62

EOM
    . $script
    cat $script
    rm $script
    echo "============================================================"
}


function env_init {
    local name="$1"
#    case "$name" in
#    esac
    return 0
}
