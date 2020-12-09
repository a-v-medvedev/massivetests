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

set -eu

[ -f ../env.sh ] && source ../env.sh || echo "WARNING: no environment file ../env.sh!"

BSCRIPTSDIR=./dbscripts

source $BSCRIPTSDIR/base.inc
source $BSCRIPTSDIR/funcs.inc
source $BSCRIPTSDIR/compchk.inc
source $BSCRIPTSDIR/envchk.inc
source $BSCRIPTSDIR/db.inc

function dnb_yaml-cpp() {
    local pkg="yaml-cpp"
    environment_check_specific "$pkg" || fatal "$pkg: environment check failed"
    local m=$(get_field "$1" 2 "=")
    local V=$(get_field "$2" 2 "=")
    any_mode_is_set "du" "$m" && du_github "jbeder" "yaml-cpp" "yaml-cpp-" "$V" "$m"
    local OPTS=""
    OPTS="$OPTS -DYAML_BUILD_SHARED_LIBS=OFF"
    OPTS="$OPTS -DYAML_CPP_BUILD_TESTS=OFF"
    OPTS="$OPTS -DYAML_CPP_BUILD_TOOLS=OFF"
    OPTS="$OPTS -DYAML_CPP_BUILD_CONTRIB=OFF"
    any_mode_is_set "bi" "$m" && bi_cmake "$pkg" "$V" ".." "$OPTS" "$m"
    this_mode_is_set "i" "$m" && make_binary_symlink "$pkg" "${V}"
    return 0
}

function dnb_argsparser() {
    local pkg="argsparser"
    environment_check_specific "$pkg" || fatal "$pkg: environment check failed"
    local m=$(get_field "$1" 2 "=")
    local V=$(get_field "$2" 2 "=")
    any_mode_is_set "du" "$m" && du_github "a-v-medvedev" "argsparser" "v" "$V" "$m"
    if any_mode_is_set "bi" "$m"; then 
        [ -f "$INSTALL_DIR/yaml-cpp.bin/include/yaml-cpp/yaml.h" ] || fatal "$pkg: installed yaml-cpp is required to build"
    fi
    local COMMANDS=""
    PARAMS="YAML_DIR=$INSTALL_DIR/yaml-cpp.bin"
    this_mode_is_set "b" "$m" && b_make "$pkg" "$V" "$COMMANDS" "clean" "$m"
    this_mode_is_set "b" "$m" && b_make "$pkg" "$V" "$COMMANDS" "$PARAMS" "$m"
    local FILES="argsparser/include/argsparser.h argsparser/libargsparser.so argsparser/libargsparser.a"
    this_mode_is_set "i" "$m" && i_direct_copy "$pkg" "$V" "$FILES" "$m"
    FILES="extensions"
    this_mode_is_set "i" "$m" && i_direct_copy "$pkg" "$V" "$FILES" "$m"
    this_mode_is_set "i" "$m" && make_binary_symlink "$pkg" "${V}"
    return 0
}

function dnb_psubmit() {
    local pkg="psubmit"
    environment_check_specific "$pkg" || fatal "pkg: environment check failed"
    local m=$(get_field "$1" 2 "=")
    local V=$(get_field "$2" 2 "=")
    any_mode_is_set "du" "$m" && du_github "a-v-medvedev" "psubmit" "v" "$V" "$m"
    if this_mode_is_set "i" "$m"; then
        local FILES=""
        cd ${pkg}-${V}.src
        FILES=$(ls -1 *.sh)
        cd $INSTALL_DIR
        i_direct_copy "$pkg" "$V" "$FILES" "$m"
        make_binary_symlink "$pkg" "${V}"
    fi
    return 0
}

function dnb_mpi-benchmarks() {
    local pkg="mpi-benchmarks"
    environment_check_specific "$pkg" || fatal "$pkg: environment check failed"
    local m=$(get_field "$1" 2 "=")
    local V=$(get_field "$2" 2 "=")
	any_mode_is_set "du" "$m" && du_github "a-v-medvedev" "mpi-benchmarks" "v" "$V" "$m"
    if this_mode_is_set "b" "$m"; then
        [ -f "$INSTALL_DIR/yaml-cpp.bin/include/yaml-cpp/yaml.h" ] || fatal "$pkg: installed yaml-cpp is required to build"
        [ -f "$INSTALL_DIR/argsparser.bin/argsparser.h" ] || fatal "$pkg: installed argsparser is required to build"
		cd "$INSTALL_DIR"
		cd "$pkg"-"$V".src/src_cpp
		cd "ASYNC/thirdparty"
        rm -f argsparser.bin yaml-cpp.bin
		ln -s "$INSTALL_DIR"/argsparser.bin .
		ln -s "$INSTALL_DIR"/yaml-cpp.bin .
        mkdir -p lib
		#cp "$INSTALL_DIR"/argsparser.bin/*.so "$INSTALL_DIR"/yaml-cpp.bin/lib/*.so lib/
		cp "$INSTALL_DIR"/argsparser.bin/*.a "$INSTALL_DIR"/yaml-cpp.bin/lib/*.a lib/
		cd "$INSTALL_DIR"/"$pkg"-"$V".src/src_cpp
		export CXXFLAGS="-IASYNC/thirdparty/argsparser.bin -IASYNC/thirdparty/yaml-cpp.bin/include "
#        export CXXFLAGS="$CXXFLAGS -I/opt/intel/compilers_and_libraries_2019.4.243/linux/mpi/intel64/include -L/opt/intel/compilers_and_libraries_2019.4.243/linux/mpi/intel64/lib -L/opt/intel/compilers_and_libraries_2019.4.243/linux/mpi/intel64/lib/release"
		make TARGET=ASYNC CXX=$MPICXX clean
		make TARGET=ASYNC CXX=$MPICXX
        cd "$INSTALL_DIR"
    fi
	FILES="src_cpp/IMB-ASYNC"
	this_mode_is_set "i" "$m" && i_direct_copy "$pkg" "$V" "$FILES" "$m"
	this_mode_is_set "i" "$m" && make_binary_symlink "$pkg" "${V}"
	if this_mode_is_set "i" "$m"; then
		cd "$pkg".bin
		cat > psubmit.opt.TEMPLATE << 'EOM'
QUEUE=__QUEUE__
QUEUE_SUFFIX=__QUEUE_SUFFIX__
NODETYPE=__NODETYPE__
TIME_LIMIT=3
TARGET_BIN=./IMB-ASYNC
INIT_COMMANDS=__INIT_COMMANDS__
INJOB_INIT_COMMANDS='__INJOB_INIT_COMMANDS__'

MPIEXEC=__MPI_SCRIPT__
BATCH=__BATCH_SCRIPT__
EOM
		template_to_psubmitopts .
		cd "$INSTALL_DIR"
fi

}

####

PACKAGES="yaml-cpp argsparser mpi-benchmarks psubmit"
VERSIONS="yaml-cpp:0.6.3 argsparser:0.0.9 mpi-benchmarks:0.0.5 psubmit:0.0.6"
TARGET_DIRS=""

started=$(date "+%s")
echo "Download and build started at timestamp: $started."
environment_check_main || fatal "Environment is not supported, exiting"
cd "$INSTALL_DIR"
set +u
override_versions="${PACKAGE_VERSIONS}"
set -u
what_to_build=$(expand_mode_string "$*" "$PACKAGES" ":dubi")
was_installed=""

LIST1=$(print_all_exec_files "$TARGET_DIRS")
#---
for pkg in $PACKAGES; do
    mode=$(mode_for_pkg ${pkg} "$what_to_build")
    version=$(version_for_pkg ${pkg} "${VERSIONS}" "${override_versions}" "${what_to_build}")
    eval dnb_${pkg} mode=$mode version=$version
    this_mode_is_set "i" "$mode" && was_installed=1
done
#---
LIST2=$(print_all_exec_files "$TARGET_DIRS" "$started")
if [ ! -z "$was_installed" ]; then
    echo -ne "----------\nExecutables before build start: (unix-time, size, name)\n$LIST1"
    echo -ne "----------\nExecutables after build finish: (unix-time, size, name)\n$LIST2"
fi
finished=$(date "+%s")
echo "----------"
echo "Full operation time: $(expr $finished - $started) seconds."

