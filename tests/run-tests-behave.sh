#/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
build_dir=${SCRIPTPATH}/../build

cd $SCRIPTPATH
venv_name=behave-pyvenv

# Create the behav py environment
python -m venv $venv_name

source $SCRIPTPATH/$venv_name/bin/activate
pip3 install behave
cd features

behave
