#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

build_dir=${SCRIPTPATH}/../build

function print_help()
{
	echo "########################################################################"
	echo "Usage"
	echo "$SCRIPT [Build directory]"
}

if [ $# -eq 1 ]; then
	if [[ ! -d "$1" ]]; then
		print_help
		echo ""
		echo "${1} is not a valid directory"
		exit 255
	fi
	build_dir=${1}
fi

echo "Executing software test suite"
echo "Test location: $build_dir"

# Tests are registered with CTest (gtest_discover_tests); a non-zero exit
# code propagates so CI fails when any test fails.
ctest --test-dir "${build_dir}" --output-on-failure
exit $?
