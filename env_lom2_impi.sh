function env_init_global {
    echo "=== Specific Environment settings for Lomonosov-2 supercomputer ==="
    script=$(mktemp .XXXXXX.sh)
cat > $script << 'EOM'
module load slurm gcc/9.1 cmake/3.12.3 intel/2019.5 
#impi/2019.4
. ~/impi2019/compilers_and_libraries/linux/mpi/intel64/bin/mpivars.sh release


export MPICC=mpigcc
export MPICXX=mpigxx

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
export PSUBMIT_OPTS_INIT_COMMANDS='source $HOME/impi2019/compilers_and_libraries/linux/mpi/intel64/bin/mpivars.sh release'
export PSUBMIT_OPTS_INJOB_INIT_COMMANDS='source $HOME/impi2019/compilers_and_libraries/linux/mpi/intel64/bin/mpivars.sh release'
export PSUBMIT_OPTS_MPI_SCRIPT=impixtmp
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
