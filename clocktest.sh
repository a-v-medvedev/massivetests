#export MASSIVETEST_AUX_ARGS="-workload calc -thread_level multiple"
export LD_LIBRARY_PATH=$PWD/thirdparty/yaml-cpp.bin/lib:$PWD/thirdparty/argsparser.bin:$LD_LIBRARY_PATH
export PATH=$PWD/psubmit.bin:$PATH

cat /dev/null > local_impi_env.sh
#echo export IMB_ASYNC_ESTIMATION_CYCLES=3 >> local_impi_env.sh 
#echo export IMB_ASYNC_AVG_OPT=AVERAGE >> local_impi_env.sh
#echo export I_MPI_ASYNC_PROGRESS=1 >> local_impi_env.sh
#echo export I_MPI_ASYNC_PROGRESS_PIN=1,3,5,7,9,11 >> local_impi_env.sh
#echo export I_MPI_PIN_PROCESSOR_LIST=0,2,4,6,8,10 >> local_impi_env.sh
./massivetest --workloads="calc_calibration:tavg" --scale="2:$PSUBMIT_OPTS_PPN" --sizes="4:100" --nqueued="1" --repeats=1 --driver=imb_async 
