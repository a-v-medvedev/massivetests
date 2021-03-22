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


function set_specific_params() {
    local MODE=$1
    local SUBMODE=$2
    if [ "$MASSIVE_TESTS_DRIVER" != "imb_async" ]; then
        fatal "MASSIVE_TESTS_DRIVER must be imb_async."
    fi
    cat /dev/null > ./local_impi_env.sh
    echo "export I_MPI_DEBUG=5" >> ./local_impi_env.sh
    export MASSIVETEST_AUX_ARGS=""
    case "$SUBMODE" in
        base)
            ;;
        avg) echo export IMB_ASYNC_AVG_OPT="AVERAGE" >> ./local_impi_env.sh
            ;;
        med) echo export IMB_ASYNC_AVG_OPT="MEDIAN" >> ./local_impi_env.sh
            ;;
        max) echo export IMB_ASYNC_AVG_OPT="MAX" >> ./local_impi_env.sh
            ;;
        prog1)
            echo export IMB_ASYNC_AVG_OPT="AVERAGE" >> ./local_impi_env.sh
            export MASSIVETEST_AUX_ARGS="-spinperiod 1" 
            ;;
        prog2)
            echo export IMB_ASYNC_AVG_OPT="AVERAGE" >> ./local_impi_env.sh
            export MASSIVETEST_AUX_ARGS="-spinperiod 2" 
            ;;
        prog5)
            echo export IMB_ASYNC_AVG_OPT="AVERAGE" >> ./local_impi_env.sh
            export MASSIVETEST_AUX_ARGS="-spinperiod 5" 
            ;;
        prog10)
            echo export IMB_ASYNC_AVG_OPT="AVERAGE" >> ./local_impi_env.sh
            export MASSIVETEST_AUX_ARGS="-spinperiod 10" 
            ;;
        prog20)
            echo export IMB_ASYNC_AVG_OPT="AVERAGE" >> ./local_impi_env.sh
            export MASSIVETEST_AUX_ARGS="-spinperiod 20" 
            ;;
        *) fatal "Unknown submode: $SUBMODE"
            ;;
    esac

    case "$MODE" in
          base)
              ;;
          none)
              export MASSIVETEST_AUX_ARGS="-workload none"
              ;;
          calc)
              export MASSIVETEST_AUX_ARGS="-workload calc"
              ;;
          calc_progress)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -workload calc_and_progress"
              ;;
          calc_mpich_progress)
              echo export I_MPI_ASYNC_PROGRESS=1 >> ./local_impi_env.sh
              export MASSIVETEST_AUX_ARGS="-workload calc -thread_level multiple"
              ;;
          *) fatal "Unknown mode: $MODE"
             ;;
    esac
}
