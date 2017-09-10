#!/bin/bash

set -eu
set -x
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR


[ 0 -ne $# ]

if [ 1 -eq $# ] ; then
	if type rsvg 2>/dev/null 1>/dev/null
	then
		rsvg ${1}
	else
		rsvg-convert ${1}
	fi

	exit 0
fi


[ 2 -eq $# ]

if type rsvg 2>/dev/null 1>/dev/null
then
	rsvg ${1} ${2}
else
	rsvg-convert -o ${2} ${1}
fi

