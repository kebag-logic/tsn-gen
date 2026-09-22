#!/bin/bash
# Usage: run.sh <id> <description> <command...>
# Runs one command, keeping its combined output in logs/<id>.log and a JSON
# record (cwd, argv, UTC start/end, exact exit status, log SHA-256) appended
# to commands.jsonl. The script exits with the command's own status.
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
id=$1
desc=$2
shift 2
log="$E/logs/$id.log"
start=$(date -u +%Y-%m-%dT%H:%M:%SZ)
{
	echo "# id: $id"
	echo "# description: $desc"
	echo "# cwd: $(pwd)"
	echo "# argv: $(printf '%q ' "$@")"
	echo "# start: $start"
} > "$log"
"$@" >> "$log" 2>&1
status=$?
end=$(date -u +%Y-%m-%dT%H:%M:%SZ)
echo "# end: $end exit: $status" >> "$log"
sha=$(sha256sum "$log" | cut -d' ' -f1)
python3 - "$E/commands.jsonl" "$id" "$desc" "$(pwd)" "$start" "$end" "$status" "logs/$id.log" "$sha" "$@" <<'PY'
import json, sys
path, cid, desc, cwd, start, end, status, log, sha, *argv = sys.argv[1:]
with open(path, "a") as f:
    f.write(json.dumps({"id": cid, "description": desc, "cwd": cwd,
                        "argv": argv, "start": start, "end": end,
                        "exit": int(status), "log": log, "log_sha256": sha}) + "\n")
PY
echo "[$id] exit=$status log=$log"
exit $status
