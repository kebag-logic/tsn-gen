#/bin/bash

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
PRECOMMIT_HOOK_PATH=$SCRIPTPATH/../.git/hooks/pre-commit
cat $PRECOMMIT_HOOK_PATH << EOF

./exec-local-test.sh

EOF

chmod +x $PRECOMMIT_HOOK_PATH
