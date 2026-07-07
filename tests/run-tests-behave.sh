#!/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
repo_root=$(realpath -s "${SCRIPTPATH}/..")

cd "$SCRIPTPATH" || exit 255
venv_name=behave-pyvenv

# Create the behave py environment
python3 -m venv $venv_name

source "$SCRIPTPATH/$venv_name/bin/activate"
pip3 install --quiet behave

# The BDD suites shell out to packet_gen; environment.py resolves the binary
# from PACKET_GEN_BIN or defaults to <repo>/build/traffic-gen/packet_gen.
# @hardware needs CAP_NET_RAW and @verilator needs a live DUT socket, so
# both are excluded here; run them explicitly on a suitable host.
status=0
for suite in \
	"${repo_root}/traffic-gen/tests/aecp_behave/features" \
	"${repo_root}/traffic-gen/tests/stack_behave/features"
do
	echo "*** behave: ${suite}"
	behave --tags=-hardware --tags=-verilator "${suite}"
	rc=$?
	if [ $rc -ne 0 ]; then
		status=$rc
	fi
done

exit $status
