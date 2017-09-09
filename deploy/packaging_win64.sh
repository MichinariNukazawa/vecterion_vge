#!/bin/bash
#
# gtk3 app cross build
# host:		Ubuntu (Ubutn16.04 LTS amd64)
# target:	Win64
#
# depend: sudo apt-get install mingw-w64 -y
# usage:	this [package postfix]
#
# Author: michinari.nukazawa@gmail.com
#

set -eu
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR


SCRIPT_DIR=$(cd $(dirname $0); pwd)
ROOT_DIR=${SCRIPT_DIR}/..

PACKAGE_POSTFIX=
if [ 1 -eq $# ] ; then
	PACKAGE_POSTFIX="-$1"
fi

TARGET_OS=win
OBJECT_DIR=object/${TARGET_OS}
CACHE_DIR=${ROOT_DIR}/library/download_cache
GTK3LIBRARY_DEV_DIR=${OBJECT_DIR}/gtk3_win64/dev
GTK3LIBRARY_RELEASE_DIR=${OBJECT_DIR}/gtk3_win64/release
LIBXML2_DIR=${OBJECT_DIR}/libxml2
BUILD_DIR=build/${TARGET_OS}

#ARCHIVE_DEV=gtk+-bundle_3.10.4-20131202_win64.zip
ARCHIVE_DEV=gtk+-bundle-dev_3.22.20-1-20170908_win64.zip
ARCHIVE_RELEASE=gtk+-bundle_3.22.20-1-20170908-bin_win64.zip


# download gkt3 library package
if [ ! -e ${CACHE_DIR}/${ARCHIVE_DEV} -o ! -s ${CACHE_DIR}/${ARCHIVE_DEV} ] ; then
	mkdir -p ${CACHE_DIR}
	wget --tries=3 --wait=5 --continue \
		https://osdn.net/downloads/users/14/14970/gtk%2B-bundle-dev_3.22.20-1-20170908_win64.zip \
		-O ${CACHE_DIR}/${ARCHIVE_DEV}
fi

if [ ! -e ${CACHE_DIR}/${ARCHIVE_RELEASE} -o ! -s ${CACHE_DIR}/${ARCHIVE_RELEASE} ] ; then
	mkdir -p ${CACHE_DIR}
	wget --tries=3 --wait=5 --continue \
		https://osdn.net/downloads/users/14/14971/gtk%2B-bundle_3.22.20-1-20170908-bin_win64.zip \
		-O ${CACHE_DIR}/${ARCHIVE_RELEASE}
fi


# decompress gtk3
if [ ! -e ${GTK3LIBRARY_DEV_DIR}/lib/pkgconfig/gtk+-3.0.pc ] ; then
	rm -rf ${GTK3LIBRARY_DEV_DIR}
	mkdir -p ${GTK3LIBRARY_DEV_DIR}
	pushd ${GTK3LIBRARY_DEV_DIR}

	unzip ${CACHE_DIR}/${ARCHIVE_DEV} > /dev/null
	find -name '*.pc' | while read pc; do sed -e "s@^prefix=.*@prefix=$PWD@" -i "$pc"; done
	find -name '*.pc' | while read pc; do sed -e "s@/srv/win32builder/fixed_3104/build/win64@$PWD@g" -i "$pc"; done

	popd
fi

if [ ! -e ${GTK3LIBRARY_RELEASE_DIR}/bin/libgtk-3-0.dll ] ; then
	rm -rf ${GTK3LIBRARY_RELEASE_DIR}
	mkdir -p ${GTK3LIBRARY_RELEASE_DIR}
	pushd ${GTK3LIBRARY_RELEASE_DIR}

	unzip ${CACHE_DIR}/${ARCHIVE_RELEASE} > /dev/null

	popd
fi

# make resource(icon and other)
TARGET_OS_WIN_RESOURCE=${OBJECT_DIR}/vecterion.res
x86_64-w64-mingw32-windres ${ROOT_DIR}/deploy/win/vecterion.rc -O coff -o ${TARGET_OS_WIN_RESOURCE}


# build app source
CFLAGS_APPEND=-Wno-missing-include-dirs

[ -d ${GTK3LIBRARY_DEV_DIR}/lib/pkgconfig ]
export PKG_CONFIG_PATH=${GTK3LIBRARY_DEV_DIR}/lib/pkgconfig
make TARGET_OS=win CC=x86_64-w64-mingw32-gcc CFLAGS_APPEND=${CFLAGS_APPEND} INCLUDE_APPEND="-mwindows" TARGET_OS_WIN_RESOURCE=${TARGET_OS_WIN_RESOURCE}



# packaging
[ 1 -eq $(cat ${OBJECT_DIR}/version.c | grep "SHOW_VERSION" | wc -l) ]
SHOW_VERSION=$(cat ${OBJECT_DIR}/version.c | grep "SHOW_VERSION" | sed -e 's/.\+=.*"\([0-9.]\+\)".\+/\1/g')
GIT_HASH=$(cat ${OBJECT_DIR}/version.c | grep "GIT_HASH" | sed -e 's/.\+=.*"\([0-9a-f]\+\)".\+/\1/g')
GIT_STATUS_SHORT=$(cat ${OBJECT_DIR}/version.c | grep "GIT_STATUS_SHORT" | sed -e 's/.\+=.*"\(.*\)".\+/\1/g')
EX=""
if [ -n "${GIT_STATUS_SHORT}" ] ; then
EX="develop"
fi
PACKAGE_NAME=vecterion_vge-win64-${SHOW_VERSION}${EX}-${GIT_HASH}${PACKAGE_POSTFIX}
PACKAGE_DIR=${BUILD_DIR}/${PACKAGE_NAME}

rm -rf ${PACKAGE_DIR}
mkdir -p ${PACKAGE_DIR}
# vecterion
cp ${BUILD_DIR}/vecterion_vge.exe ${PACKAGE_DIR}/
cp -r ${ROOT_DIR}/resource ${PACKAGE_DIR}/
cp ${ROOT_DIR}/deploy/\!cons.bat ${PACKAGE_DIR}/
cp ${ROOT_DIR}/deploy/win/README_win_ja.txt ${PACKAGE_DIR}/
# library
cp -r ${GTK3LIBRARY_RELEASE_DIR}/bin/ ${PACKAGE_DIR}/
# pixbuf loader
mkdir -p ${PACKAGE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders/
cp -r ${GTK3LIBRARY_RELEASE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders/ ${PACKAGE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/
cp ${ROOT_DIR}/deploy/win/loaders.cache ${PACKAGE_DIR}/lib/gdk-pixbuf-2.0/2.10.0/
# share(icons, ...etc)
cp -r ${GTK3LIBRARY_RELEASE_DIR}/share/ ${PACKAGE_DIR}/
rm -rf ${PACKAGE_DIR}/share/doc
rm -rf ${PACKAGE_DIR}/share/locale
# fix gtk_file_chooser_dialog crush issue
#  gschemas.compiled generate:
#  `bin\glib-compile-schemas.exe share\glib-2.0\schemas` in windows
mkdir -p ${PACKAGE_DIR}/share/glib-2.0/schemas/
cp -r ${ROOT_DIR}/deploy/win/gschemas.compiled ${PACKAGE_DIR}/share/glib-2.0/schemas/

pushd ${PACKAGE_DIR}

rm -f ${ROOT_DIR}/${BUILD_DIR}/${PACKAGE_NAME}.zip
zip -r9 ${ROOT_DIR}/${BUILD_DIR}/${PACKAGE_NAME}.zip *

popd

