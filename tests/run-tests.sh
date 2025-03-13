#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

build_dir=${SCRIPTPATH}/../build

function print_help()
{
	clear
	echo "########################################################################"
	echo "Usage"
	echo "$SCRIPTPATH [Build directory]"
}

function subdir_tests_exec()
{
	tests=$(find ${1} -type f -executable -print)
	for stest in $tests 
	do
		echo "*** Executing ${stest}"
		$stest
		if [ $? -ne 0 ]; then
			echo "*** Test ${stest} FAILED!!!!"
		fi
	done
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

tests_folders=$(find ${build_dir} -type d -path ${build_dir}/external -prune , -iname "tests")
echo $tests_folders

for tests_folder in $tests_folders
do
	echo "** Test folder $tests_folder"
	subdir_tests_exec $tests_folder
done
