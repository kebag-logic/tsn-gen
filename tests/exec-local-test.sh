#!/bin/bash

#This scrips should be executed in the pre-commit

# TODO seperate the different action, such as the configuration and the compilation

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
PWD_CUR=$(pwd)
cd $SCRIPTPATH/act
if [ ! -d "./dist/" ]; then
	make build
fi
cd $SCRIPTPATH/..

# Execute act for local build
./tests/act/dist/local/act -P https://github.com/catthehacker/docker_images --rm
