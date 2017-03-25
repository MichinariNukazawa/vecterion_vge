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

#SCRIPT_DIR=$(cd $(dirname $0); pwd)

[ 1 -eq $# ]
VECTERION=$1



OBJECT_DIR=object/svg_read_invalid_test
SOURCE_SVG_FILE=library/23.svg

mkdir -p ${OBJECT_DIR}

# invalid element.
rm -f ${OBJECT_DIR}/out.png
set +e
${VECTERION} -s -i ${SOURCE_SVG_FILE} -o ${OBJECT_DIR}/out.png > /dev/null
RET=$?
set -e
[ 0 -ne $RET ]
[ ! -e ${OBJECT_DIR}/out.png ]

