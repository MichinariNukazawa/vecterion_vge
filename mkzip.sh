#!/bin/bash

NAME="etaion"

if [ -d ./${NAME} ] ; then
	echo "error: ./sweepが既に存在する" 
	exit
fi
git clone . ./${NAME}

zip -r9 ${NAME}_$(date '+%Y%m%d_%Hh%Mm').zip ./${NAME} > /dev/null

rm -rf ./${NAME}


