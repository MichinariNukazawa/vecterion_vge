#!/bin/bash
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



set +e
${VECTERION} -s > /dev/null
RET=$?
set -e
[ 0 -ne $RET ]

