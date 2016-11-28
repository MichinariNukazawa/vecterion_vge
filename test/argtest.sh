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
OUTPUT_FILE=${ARGTEST_OUTPUT_BASE}.svg
./${TARGET} -i ${SOURCE_FILE} -o ${OUTPUT_FILE}
[ -s ${OUTPUT_FILE} ] # file is not zero size
STR=`file ${OUTPUT_FILE}` ; [[ "${STR}" =~ "SVG" ]] # file type
###### content check (tiny. text matching base)
grep 'width="800.0' ${OUTPUT_FILE} > /dev/null
grep 'height="800.0' ${OUTPUT_FILE} > /dev/null

# jpg
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.jpg
[ -s ${ARGTEST_OUTPUT_BASE}.jpg ]
STR=`file ${ARGTEST_OUTPUT_BASE}.jpg` ; [[ "${STR}" =~ "JPEG" ]]
STR=`file ${ARGTEST_OUTPUT_BASE}.jpg` ; [[ "${STR}" =~ "800x800" ]]
# png
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.png
[ -s ${ARGTEST_OUTPUT_BASE}.png ]
STR=`file ${ARGTEST_OUTPUT_BASE}.png` ; [[ "${STR}" =~ "PNG" ]]
STR=`file ${ARGTEST_OUTPUT_BASE}.png` ; [[ "${STR}" =~ "800 x 800" ]]
STR=`file ${ARGTEST_OUTPUT_BASE}.png` ; [[ "${STR}" =~ "RGBA" ]]
# bmp
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.bmp
[ -s ${ARGTEST_OUTPUT_BASE}.bmp ]
STR=`file ${ARGTEST_OUTPUT_BASE}.bmp` ; [[ "${STR}" =~ "PC bitmap" ]]
STR=`file ${ARGTEST_OUTPUT_BASE}.bmp` ; [[ "${STR}" =~ "800 x 800" ]]

# jpeg (alias jpg)
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}.jpeg
[ -s ${ARGTEST_OUTPUT_BASE}.jpeg ]
STR=`file ${ARGTEST_OUTPUT_BASE}.jpeg` ; [[ "${STR}" =~ "JPEG" ]]
# PNG (alias png)
./${TARGET} -i ${SOURCE_FILE} -o ${ARGTEST_OUTPUT_BASE}_02.PNG
[ -s ${ARGTEST_OUTPUT_BASE}_02.PNG ]
STR=`file ${ARGTEST_OUTPUT_BASE}_02.PNG` ; [[ "${STR}" =~ "PNG" ]]

