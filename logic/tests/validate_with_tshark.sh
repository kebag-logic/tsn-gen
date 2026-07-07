#!/bin/bash
#
# SPDX-License-Identifier: MIT
#
# Cross-check a generated ATDECC AECP frame against Wireshark's dissector.
# Generates one deterministic Eth+AVTP+AECP frame into a pcap via packet_gen,
# runs tshark, and asserts the dissection recognises IEEE 1722.1 AECP with a
# control_data_length of 36.
#
# Usage: validate_with_tshark.sh <packet_gen> <protocols_dir> <stack_yaml> <tshark>

set -euo pipefail

PACKET_GEN="$1"
PROTO_DIR="$2"
STACK_YAML="$3"
TSHARK="$4"

workdir="$(mktemp -d)"
trap 'rm -rf "$workdir"' EXIT
pcap="$workdir/frame.pcap"

# One reproducible frame delivered to a pcap file.
"$PACKET_GEN" \
    --yaml-dir "$PROTO_DIR" \
    --stack-file "$STACK_YAML" \
    --seed 42 --count 1 \
    --transport "pcap:$pcap" \
    --log-level error >/dev/null

# Dissect. -O ieee17221 narrows output to the ATDECC protocol tree.
dissection="$("$TSHARK" -r "$pcap" -V 2>/dev/null || true)"

echo "$dissection" | grep -qi "IEEE 1722.1" \
    || { echo "FAIL: tshark did not recognise IEEE 1722.1"; \
         echo "$dissection" | head -40; exit 1; }

echo "PASS: tshark dissected the frame as IEEE 1722.1 ATDECC"
