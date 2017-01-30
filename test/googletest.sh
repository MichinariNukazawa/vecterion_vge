#!/bin/bash
#
# Author: michinari.nukazawa@gmail.com
#

set -eu
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR


SCRIPT_DIR=$(cd $(dirname $0); pwd)
ROOT_DIR=${SCRIPT_DIR}/..
LIBRARY_DIR=${ROOT_DIR}/library
CACHE_DIR=${LIBRARY_DIR}/download_cache



# # download
if [ ! -e ${CACHE_DIR}/googletest.zip ] ; then
	mkdir -p ${CACHE_DIR}
	wget --tries=3 --wait=5 --continue \
		https://github.com/google/googletest/archive/master.zip \
		-P ${CACHE_DIR}
	mv ${CACHE_DIR}/master.zip ${CACHE_DIR}/googletest.zip
fi

# # decompress
mkdir -p ${LIBRARY_DIR}
pushd ${LIBRARY_DIR}

[ ! -d googletest/ ] # skip if directory already exist

unzip ${CACHE_DIR}/googletest.zip > /dev/null
mv googletest-master googletest

# build
pushd googletest/googletest

cmake CMakeLists.txt
make

popd

popd


