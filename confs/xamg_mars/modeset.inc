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
    if [ "$MASSIVE_TESTS_DRIVER" != "functest" ]; then
        fatal "MASSIVE_TESTS_DRIVER must be functest."
    fi
    case "$SUBMODE" in
        base)
            ;;
        solve)
            export MASSIVETEST_AUX_ARGS="-mode solver"
            ;;
        spmv)
            export MASSIVETEST_AUX_ARGS="-mode spmv"
            ;;
        hypre)
            export MASSIVETEST_AUX_ARGS="-mode hypre"
            ;;

        *) fatal "Unknown submode: $SUBMODE"
            ;;
    esac

    case "$MODE" in
          base)
              ;;
              1:1)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -node_config nnodes=1:ncores=1"
              ;;
              1:2)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -node_config nnodes=1:ncores=2"
              ;;
              1:4)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -node_config nnodes=1:ncores=4"
              ;;
              2:1)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -node_config nnodes=2:ncores=1"
              ;;
              2:2)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -node_config nnodes=2:ncores=2"
              ;;
              4:1)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -node_config nnodes=4:ncores=1"
              ;;
          *) fatal "Unknown mode: $MODE"
             ;;
    esac
}
