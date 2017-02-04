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
if [ ! -e ${CACHE_DIR}/release-1.8.0.tar.gz ] ; then
	mkdir -p ${CACHE_DIR}
	wget --tries=3 --wait=5 --continue \
		https://github.com/google/googletest/archive/master.zip \
		-P ${CACHE_DIR}
	[ -e ${CACHE_DIR}/release-1.8.0.tar.gz ]
	mv ${CACHE_DIR}/release-1.8.0.tar.gz ${CACHE_DIR}/googletest.tar.gz
fi

# # decompress
mkdir -p ${LIBRARY_DIR}
pushd ${LIBRARY_DIR}

[ ! -d googletest/ ] # skip if directory already exist

tar zxvf ${CACHE_DIR}/googletest.tar.gz > /dev/null
mv googletest-release-1.8.0 googletest

# build
pushd googletest/googletest

cmake CMakeLists.txt
make

popd

popd


