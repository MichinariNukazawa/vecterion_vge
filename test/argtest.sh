#
#
#

set -ue
set -x

trap 'echo "$0(${LINENO}) ${BASH_COMMAND}"' ERR

[ 3 -eq $# ]

TARGET=$1
SOURCE_FILE=$2
ARGTEST_OUTPUT_BASE=$3

[ -n ${ARGTEST_OUTPUT_BASE} ]

rm -rf ${ARGTEST_OUTPUT_BASE}.*
mkdir -p `dirname ${ARGTEST_OUTPUT_BASE}`

# svg
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.svg
[ -s ${ARGTEST_OUTPUT_BASE}.svg ] # file is not zero size
STR=`file ${ARGTEST_OUTPUT_BASE}.svg` ; [[ "${STR}" =~ "SVG" ]] # file type
# jpg
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.jpg
[ -s ${ARGTEST_OUTPUT_BASE}.jpg ]
STR=`file ${ARGTEST_OUTPUT_BASE}.jpg` ; [[ "${STR}" =~ "JPEG" ]]
# png
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.png
[ -s ${ARGTEST_OUTPUT_BASE}.png ]
STR=`file ${ARGTEST_OUTPUT_BASE}.png` ; [[ "${STR}" =~ "PNG" ]]
# bmp
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.bmp
[ -s ${ARGTEST_OUTPUT_BASE}.bmp ]
STR=`file ${ARGTEST_OUTPUT_BASE}.bmp` ; [[ "${STR}" =~ "PC bitmap" ]]

# jpeg (alias jpg)
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.jpeg
[ -s ${ARGTEST_OUTPUT_BASE}.jpeg ]
STR=`file ${ARGTEST_OUTPUT_BASE}.jpeg` ; [[ "${STR}" =~ "JPEG" ]]
# PNG (alias png)
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}_02.PNG
[ -s ${ARGTEST_OUTPUT_BASE}_02.PNG ]
STR=`file ${ARGTEST_OUTPUT_BASE}_02.PNG` ; [[ "${STR}" =~ "PNG" ]]

