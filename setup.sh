#!/bin/bash

# TODO seperate the different action, such as the configuration and the compilation

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

# 1st Configure
if [ $# -eq 1 ]; then
	if [[ "${1}" == "d" ]]; then
		rm -r ./build
	else
		DATE=$(date '+%Y%m%d_%H%m%S%3N')
		mv ./build ./build_${DATE}
	fi
fi

mkdir -p ${SCRIPTPATH}/build
cd ${SCRIPTPATH}/build

## Configuration
cmake ${SCRIPTPATH}
if [ $? -ne 0 ]; then
	echo "ERROR: Configuration failed"
	exit 255
fi

## Compilation
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "ERROR: Compilation failed"
	exit 255
fi

../tests/run-tests.sh

cd ${SCRIPTPATH}
