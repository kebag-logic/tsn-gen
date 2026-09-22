#!/bin/bash
# Usage: bdd-suites.sh <id-prefix> <source-dir> <packet_gen>
# Runs both documented Behave suites from <source-dir> against <packet_gen>
# with the stock runner's exclusions (--tags=-hardware --tags=-verilator),
# each as its own run.sh record so both suite exits are kept. The fixed /tmp
# pcap paths the features write are listed before and after, so a collision
# with another user of those paths would be visible.
set -u
E=$WORKSPACE_HOME/milan-fpga-management/2026-09-22/tsngen15-author
S=$VALIDATION_STORAGE/tmp/a162-tsngen15
id=$1
src=$2
bin=$3
list_tmp() {
	ls -l --time-style=full-iso /tmp/aecp_*.pcap /tmp/stack_*.pcap /tmp/rt_*.pcap 2>/dev/null || echo "(none)"
}
list_tmp > "$E/logs/$id-tmp-before.txt"
cd "$src" || exit 255
status=0
for suite in aecp_behave stack_behave; do
	PACKET_GEN_BIN="$bin" "$E/scripts/run.sh" "$id-$suite" \
		"behave --tags=-hardware --tags=-verilator traffic-gen/tests/$suite/features/ with PACKET_GEN_BIN=$bin" \
		"$S/venv/bin/behave" --tags=-hardware --tags=-verilator "traffic-gen/tests/$suite/features/"
	rc=$?
	[ $rc -ne 0 ] && status=$rc
done
list_tmp > "$E/logs/$id-tmp-after.txt"
exit $status
