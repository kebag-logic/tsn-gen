#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

list_prep_scripts=$(find $SCRIPTPATH/test_resources/ -name "prep-test.sh")

for script in $list_prep_scripts
do
	 $script
done
