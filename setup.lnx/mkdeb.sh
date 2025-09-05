#!/bin/sh

#set -x

if [ "$#" != "4" ]; then
	echo "usage: `basename $0` <path-to-source-directory> <path-to-build-directory> <architecture> <path-to-output-directory>"
	exit 1
fi

SRC="$1"
BUILD="$2"
ARCH="$3"
OUT="$4"
STMP=/tmp/lumeter_${VER}_tmp
DEBNAME=""
CWD=`pwd`
VER=`cat ${SRC}/version`

copyfile()
{
    from=$1
    to=$2
	if [ ! -f $from ]; then
		echo "can not find $from"
		exit 1
	fi
	cp $from $to
	if [ "$?" != "0" ]; then
		exit 1
	fi
}

copydir()
{
    from=$1
    to=$2
	if [ ! -d $from ]; then
		echo "can not find $from"
		exit 1
	fi
	cp -r $from $to
	if [ "$?" != "0" ]; then
		exit 1
	fi
}

lumeter_PREFIX=usr

mkdeb()
{
	DEBNAME="lumeter_${VER}_${ARCH}.deb"

	echo making structure of $DEBNAME in ${STMP}...
	rm -rf $STMP
	mkdir $STMP
	cd $STMP

	mkdir -p ${lumeter_PREFIX}
	cd ${lumeter_PREFIX}
	mkdir -p bin

	copyfile ${BUILD}/lumeter ./bin
	strip ./bin/lumeter

	echo "creating DEB controls..."
	cd $STMP
	mkdir DEBIAN
	cat << EOF >> DEBIAN/control
Package: lumeter
Version: $VER
Section: misc
Priority: optional
Architecture: ${ARCH}
Depends:
Replaces:
Installed-Size: `du -sh . | sed 's/[^0-9]//g'`
Maintainer: Sergej Bauer <sergej.bauer@gmail.com>
Description: application for measuring speed on network interfaces of various machines.
EOF
	cd $OUT
	pwd
	dpkg -b $STMP $DEBNAME
	cd $CWD
}

mkdeb
touch setup.stamp
