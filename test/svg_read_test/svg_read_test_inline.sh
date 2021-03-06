#!/bin/bash
#
# Author: michinari.nukazawa@gmail.com
#

set -eu
set -x
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR

SCRIPT_DIR=$(cd $(dirname $0); pwd)

[ 3 -eq $# ]
VECTERION=$1
LOG_DIR=$2
SOURCE_SVG_FILE=$3



FILENAME=$(basename "$SOURCE_SVG_FILE")
NAME="${FILENAME%.*}"

mkdir -p ${LOG_DIR}

bash ${SCRIPT_DIR}/rsvg.sh ${SOURCE_SVG_FILE} ${LOG_DIR}/${NAME}_rsvg.png

# raster output(png)
${VECTERION} -s -i ${SOURCE_SVG_FILE} -o ${LOG_DIR}/${NAME}_vecterion.png >> ${LOG_DIR}/log_.log
compare -verbose -metric AE ${LOG_DIR}/${NAME}_rsvg.png ${LOG_DIR}/${NAME}_vecterion.png ${LOG_DIR}/${NAME}_diff_AE_png.png

# vector output(svg)
${VECTERION} -s -i ${SOURCE_SVG_FILE} -o ${LOG_DIR}/${NAME}_vecterion.svg >> ${LOG_DIR}/log_.log
bash ${SCRIPT_DIR}/rsvg.sh ${LOG_DIR}/${NAME}_vecterion.svg ${LOG_DIR}/${NAME}_vecterion_rsvg.png
compare -verbose -metric AE ${LOG_DIR}/${NAME}_rsvg.png ${LOG_DIR}/${NAME}_vecterion_rsvg.png ${LOG_DIR}/${NAME}_diff_AE_svg.png

# vector re input(svg)
${VECTERION} -s -i ${LOG_DIR}/${NAME}_vecterion.svg -o ${LOG_DIR}/${NAME}_vecterion_vecterion.png >> ${LOG_DIR}/log_.log
compare -verbose -metric AE ${LOG_DIR}/${NAME}_rsvg.png ${LOG_DIR}/${NAME}_vecterion_vecterion.png ${LOG_DIR}/${NAME}_diff_AE_svg_svg.png

