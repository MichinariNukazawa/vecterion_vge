#!/bin/bash
#
# depend: sudo apt-get install rsvg -y
#
# Author: michinari.nukazawa@gmail.com
#

set -eu
set -x
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR

SCRIPT_DIR=$(cd $(dirname $0); pwd)

[ 1 -eq $# ]
VECTERION=$1



GIT_HASH=$(git log --pretty=format:'%h' -n 1)
GIT_STATUS_SHORT=$(git diff --stat | tail -1)
GIT_TIMESTAMP=$(git log -n1 --format="%at")
MAJOR_MINER_VERSION=$(date -u -d @${GIT_TIMESTAMP} +%Y.%m%d%H%M%S)
STAT=""
if [ -n "${GIT_STATUS_SHORT}" ] ; then
	STAT="+"
fi
VERSION=${MAJOR_MINER_VERSION}-${GIT_HASH}${STAT}

DATA_DIR=${SCRIPT_DIR}

LOG_DIR=log/svg_read_test/${VERSION}
mkdir -p ${LOG_DIR}

rsvg --version > ${LOG_DIR}/log.log

function svg_read_test(){
	# echo -e "file:${1}"
	bash ${SCRIPT_DIR}/svg_read_test_inline.sh ${VECTERION} ${LOG_DIR} ${1}
}

for SOURCE_SVG_FILE in $(find ${DATA_DIR} -name *svg) ; do
	svg_read_test ${SOURCE_SVG_FILE}
done

