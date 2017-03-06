#!/bin/bash
#
# gtk3 app cross build
# host:		Ubuntu (Ubutn16.04 LTS amd64)
# target:	Win64
#
# depend: sudo apt-get install mingw-w64 -y
#
# Author: michinari.nukazawa@gmail.com
#

set -eu
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR


SCRIPT_DIR=$(cd $(dirname $0); pwd)
ROOT_DIR=${SCRIPT_DIR}/..

TARGET_ARCH=win
OBJECT_DIR=object/${TARGET_ARCH}
CACHE_DIR=${ROOT_DIR}/library/download_cache
GTK3LIBRARY_DIR=${OBJECT_DIR}/gtk3_win64
LIBXML2_DIR=${OBJECT_DIR}/libxml2
BUILD_DIR=build/${TARGET_ARCH}


# download gkt3 library binary
if [ ! -e ${CACHE_DIR}/gtk+-bundle_3.10.4-20131202_win64.zip ] ; then
	mkdir -p ${CACHE_DIR}
	wget --tries=3 --wait=5 --continue \
		http://win32builder.gnome.org/gtk+-bundle_3.10.4-20131202_win64.zip \
		-P ${CACHE_DIR}
fi


# decompress gtk3
if [ ! -e ${GTK3LIBRARY_DIR}/lib/pkgconfig/gtk+-3.0.pc ] ; then
	rm -rf ${GTK3LIBRARY_DIR}
	mkdir -p ${GTK3LIBRARY_DIR}
	pushd ${GTK3LIBRARY_DIR}

	unzip ${CACHE_DIR}/gtk+-bundle_3.10.4-20131202_win64.zip > /dev/null
	find -name '*.pc' | while read pc; do sed -e "s@^prefix=.*@prefix=$PWD@" -i "$pc"; done
	find -name '*.pc' | while read pc; do sed -e "s@/srv/win32builder/fixed_3104/build/win64@$PWD@g" -i "$pc"; done
	sed -e "s@Z:/srv/win32builder/fixed_3104/build/win32/@@g" -i "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache"

	popd
fi


# make resource(icon and other)
TARGET_ARCH_WIN_RESOURCE=${OBJECT_DIR}/vecterion.res
x86_64-w64-mingw32-windres ${ROOT_DIR}/deploy/win/vecterion.rc -O coff -o ${TARGET_ARCH_WIN_RESOURCE}


# build app source
CFLAGS_APPEND=-Wno-missing-include-dirs

[ -d ${GTK3LIBRARY_DIR}/lib/pkgconfig ]
export PKG_CONFIG_PATH=${GTK3LIBRARY_DIR}/lib/pkgconfig
make TARGET_ARCH=win CC=x86_64-w64-mingw32-gcc CFLAGS_APPEND=${CFLAGS_APPEND} INCLUDE_APPEND="-mwindows" TARGET_ARCH_WIN_RESOURCE=${TARGET_ARCH_WIN_RESOURCE}



# packaging
[ 1 -eq $(cat ${OBJECT_DIR}/version.c | grep "SHOW_VERSION" | wc -l) ]
SHOW_VERSION=$(cat ${OBJECT_DIR}/version.c | grep "SHOW_VERSION" | sed -e 's/.\+=.*"\([0-9.]\+\)".\+/\1/g')
GIT_HASH=$(cat ${OBJECT_DIR}/version.c | grep "GIT_HASH" | sed -e 's/.\+=.*"\([0-9a-f]\+\)".\+/\1/g')
GIT_STATUS_SHORT=$(cat ${OBJECT_DIR}/version.c | grep "GIT_STATUS_SHORT" | sed -e 's/.\+=.*"\(.*\)".\+/\1/g')
EX=""
if [ -n "${GIT_STATUS_SHORT}" ] ; then
EX="develop"
fi
PACKAGE_NAME=vecterion_vge-win64-${SHOW_VERSION}${EX}-${GIT_HASH}
PACKAGE_DIR=${BUILD_DIR}/${PACKAGE_NAME}

rm -rf ${PACKAGE_DIR}
mkdir -p ${PACKAGE_DIR}
cp ${BUILD_DIR}/vecterion_vge.exe ${PACKAGE_DIR}/
cp -r ${ROOT_DIR}/resource ${PACKAGE_DIR}/
cp ${ROOT_DIR}/deploy/\!cons.bat ${PACKAGE_DIR}/
cp ${GTK3LIBRARY_DIR}/bin/*.dll ${PACKAGE_DIR}/
#cp ${GTK3LIBRARY_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.dll ${PACKAGE_DIR}/
mkdir -p ${PACKAGE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders/
cp ${GTK3LIBRARY_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.dll ${PACKAGE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders/
cp ${GTK3LIBRARY_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache ${PACKAGE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/
mkdir -p ${PACKAGE_DIR}/share/glib-2.0/
cp -r ${GTK3LIBRARY_DIR}/share/glib-2.0/schemas ${PACKAGE_DIR}/share/glib-2.0/

pushd ${PACKAGE_DIR}

zip -r9 ${ROOT_DIR}/${BUILD_DIR}/${PACKAGE_NAME}.zip *

popd

