#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

while true
do
	clear
	inotifywait -qrm --event modify --format '%w%f' ${SCRIPTPATH}/../parser/ | 
	while read -r file ; do
		echo "Changes in $file"
		${SCRIPTPATH}/../setup.sh
		if [ $? -ne 0 ]; then
			continue
		fi 

		${SCRIPTPATH}/run-tests.sh
	done
done
