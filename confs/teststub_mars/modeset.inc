#!/bin/bash
#
#   This file is part of massivetest.
#
#   Massivetest is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    massivetests is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with massivetests.  If not, see <https://www.gnu.org/licenses/>.
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
        always)
            export MASSIVETEST_AUX_ARGS="-mode always"
            ;;
        never)
            export MASSIVETEST_AUX_ARGS="-mode never"
            ;;

        *) fatal "Unknown submode: $SUBMODE"
            ;;
    esac

    case "$MODE" in
          base)
              ;;
          T)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -code T"
              ;;
          A)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -code A"
              ;;
          C)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -code C"
              ;;
          E)
              export MASSIVETEST_AUX_ARGS="$MASSIVETEST_AUX_ARGS -code E"
              ;;
          *) fatal "Unknown mode: $MODE"
             ;;
    esac
}
