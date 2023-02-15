#!/bin/bash

RED='\e[1;31m'
GRN='\e[1;32m'
RST='\e[0m'

warn()
{
	echo -e $RED"$@"$RST
}

die()
{
	warn "$@"
	exit 1
}

set -eE
trap "die 'An error occured, check output above.'" ERR

CMAKE=${CMAKE:-$(command -v cmake || true)}
export SRC_DIR="$(readlink -f "$(dirname "$0")")/soccer"
export CTC_DIR="$SRC_DIR/../ctc"

[ -x "$CMAKE" ] || die "CMake not found, please install cmake first."

usage()
{
	cat <<-EOF
	usage: $0 [OPTIONS]
	  -G "<CMAKE_GENERATOR_NAME>"
	      Set's the generator name for cmake. Run 'cmake -G' for a full list.

	      Some often used generator names are:
	      Unix Makefiles                = Generates standard UNIX makefiles.
	      CodeBlocks - Unix Makefiles   = Generates CodeBlocks project files.
	      Eclipse CDT4 - Unix Makefiles = Generates Eclipse CDT 4.0 project files.

	  --no-ccache
	      Disable usage of ccache (not recommended)

	  --no-local
	      Don't setup local compiler for behavior simulator

	EOF
}

cmake_config_header()
{
	local TARGET=$1
	echo ""
	echo "...:::: Configuration - bembelbots-framework [$TARGET] ::::..."
}

build6()
{
	# set up build directory
	local BUILD_DIR="${SRC_DIR}/../build"
	
	rm -rf "${BUILD_DIR}"
	mkdir -p "${BUILD_DIR}"
	
	pushd "${BUILD_DIR}"
	cmake_config_header "V6"
	$CMAKE -Wno-dev -DCTC_DIR="${CTC_DIR}" \
		-DCMAKE_SYSROOT="${CTC_DIR}/ctc-linux64-atom-2.8.5.10/yocto-sdk/sysroots/core2-32-sbr-linux" \
		-DCMAKE_MODULE_PATH="${CTC_DIR}/ctc-linux64-atom-2.8.5.10/yocto-sdk/sysroots/core2-32-sbr-linux/usr/share/cmake-3.4" \
		-DCMAKE_TOOLCHAIN_FILE="${SRC_DIR}/cmake/v6toolchain.cmake" \
		-DCMAKE_BUILD_TYPE=Debug \
		-DV6=ON "$SRC_DIR"
	popd
}

[ -x "${CTC_DIR}/gcc/bin/i686-bembelbots-linux-gnu-g++" ] || die "No cross compiler found, please follow the setup guide in BUILD.md"
[ -d "$CTC_DIR/ctc-linux64-atom-2.8.5.10/yocto-sdk/sysroots/core2-32-sbr-linux" ] || die "Nao V6 sysroot not found, please follow the setup guide in BUILD.md"

cd "$CTC_DIR"
[ -e "ctc-linux64-atom-2.8.5.10/.patched" ] || patch -p1 < ctc_missing_opencv_ts.patch

cd "$SRC_DIR"
build6
echo -e "${GRN}All done. Change to \"build/\" and run 'make' now.${RST}"
