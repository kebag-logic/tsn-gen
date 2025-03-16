#/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
PRECOMMIT_HOOK_PATH=$SCRIPTPATH/../.git/hooks/pre-commit
cat << EOF > $PRECOMMIT_HOOK_PATH
./tests/exec-local-test.sh
EOF

chmod +x $PRECOMMIT_HOOK_PATH
