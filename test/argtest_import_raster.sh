#
#
#

set -ue
set -x

trap 'echo "$0(${LINENO}) ${BASH_COMMAND}"' ERR

[ 3 -eq $# ]

TARGET=$1
SOURCE_DIR=$2
ARGTEST_OUTPUT_BASE=$3

[ -n ${ARGTEST_OUTPUT_BASE} ]

#### jpeg
./${TARGET} -i ${SOURCE_DIR}/daisy_bell_header_r2.jpg -o ${ARGTEST_OUTPUT_BASE}_dbh.svg
[ -s ${ARGTEST_OUTPUT_BASE}_dbh.svg ] # file is not zero size
STR=`file ${ARGTEST_OUTPUT_BASE}.svg` ; [[ "${STR}" =~ "SVG" ]] # file type

