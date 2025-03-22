#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

level=0
function recursion_gen()
{
	local j

	if [ $level == 4 ]; then
		cd ..
		return
	fi

	for j in {0..2}
	do
		mkdir -p tf_${level}-${j}
		cd tf_${level}-${j}
		level=$((level + 1))
		recursion_gen
		level=$((level - 1))
	done
	cd ..
}

cd $SCRIPTPATH
recursion_gen
