#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

PROJECT=${1}

while true
do
	clear
	inotifywait -qrm --event modify --format '%w%f' ${SCRIPTPATH}/../${PROJECT}/ |
	while read -r file ; do
		echo "Changes in $file"
		${SCRIPTPATH}/../setup.sh
		if [ $? -ne 0 ]; then
			continue
		fi 

		${SCRIPTPATH}/run-tests.sh
	done
done
