#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

# 1st Configure
DATE=$(date '+%Y%m%d_%H%m%S%3N')
if [ $# -eq 1 ]; then
	 mv ./build ./build_${DATE}
fi

mkdir -p ${SCRIPTPATH}/build
cd ${SCRIPTPATH}/build
cmake ${SCRIPTPATH}
make -j$(nprocs)
../tests/run-tests.sh

cd ${SCRIPTPATH}

