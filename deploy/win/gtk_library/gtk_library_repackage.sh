#!/bin/bash
# @file
# @brief repackaging gtk binary package by @niloufarjp (https://github.com/niloufar)
#
# @author michinari.nukazawa@gmail.com
#

set -eu
set -x
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR

# base packages url
# https://box.yahoo.co.jp/guest/viewer?sid=box-l-m4xqci277itzhqw6phs2dmo7pi-1001&uniqid=891184c1-d3f2-451d-9d01-a42fa522643c#du%3D891184c1-d3f2-451d-9d01-a42fa522643c%26ds%3Dbox-l-m4xqci277itzhqw6phs2dmo7pi-1001%26tu%3Db4b424b3-bd5a-4fb3-9c0a-2ceeb0068b4d%26ts%3Dbox-l-m4xqci277itzhqw6phs2dmo7pi-1001%26vt%3Dpublic%26lf%3Dlist%26ls%3D1%26lm%3D20%26id%3D1

# need manual download
DEV_AR=gtk+-bundle-dev_3.22.20-1-20170908_win64.7z
# need manual download
REL_AR=gtk+-bundle_3.22.20-1-20170908-bin_win64.7z

# output
RET_AR=gtk+-bundle_3.22.20-1-20170908_win64.zip

SCRIPT_DIR=$(cd $(dirname $0); pwd)

pushd ${SCRIPT_DIR}

rm -rf ${DEV_AR%.*}_0
rm -rf ${REL_AR%.*}_0

mkdir ${DEV_AR%.*}_0
mkdir ${REL_AR%.*}_0

7za x -y -o"${DEV_AR%.*}_0" ${DEV_AR}
7za x -y -o"${REL_AR%.*}_0" ${REL_AR}

rm -rf ${DEV_AR%.*}
rm -rf ${REL_AR%.*}

mkdir ${DEV_AR%.*}
mkdir ${REL_AR%.*}

find "./${DEV_AR%.*}_0/" -name "*.7z" | xargs -P 1 -I % 7za x -y -o"${DEV_AR%.*}" %
find "./${REL_AR%.*}_0/" -name "*.7z" | xargs -P 1 -I % 7za x -y -o"${REL_AR%.*}" %

cp PackageInformation.md ${DEV_AR%.*}/
cp PackageInformation.md ${REL_AR%.*}/

pushd ${DEV_AR%.*}
rm -rf ../${DEV_AR%.*}.zip
zip -r9 ../${DEV_AR%.*}.zip *
popd

pushd ${REL_AR%.*}
rm -rf ../${REL_AR%.*}.zip
zip -r9 ../${REL_AR%.*}.zip *
popd

# all in one
rm -rf ${RET_AR%.*}
mkdir ${RET_AR%.*}

cp -r ${DEV_AR%.*}/* ${RET_AR%.*}
cp -r ${REL_AR%.*}/* ${RET_AR%.*}

pushd ${RET_AR%.*}
rm -rf ../${RET_AR}
zip -r9 ../${RET_AR} *
popd

popd

