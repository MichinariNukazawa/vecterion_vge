#
#
#

set -ue
set -x

trap 'echo "$0(${LINENO}) ${BASH_COMMAND}"' ERR

[ 2 -eq $# ]

TARGET=$1
ARGTEST_OUTPUT_BASE=$2

[ -n ${ARGTEST_OUTPUT_BASE} ]

#### jpeg
OUTPUT_FILE=${ARGTEST_OUTPUT_BASE}_dbh.svg
./${TARGET} -i "resource/vecterion/daisy_bell_header_r2.jpg" -o ${OUTPUT_FILE}
[ -s ${OUTPUT_FILE} ] # file is not zero size
STR=`file ${OUTPUT_FILE}` ; [[ "${STR}" =~ "SVG" ]] # file type
###### content check (tiny. text matching base)
grep 'width="960.0' ${OUTPUT_FILE} > /dev/null
grep 'height="500.0' ${OUTPUT_FILE} > /dev/null

