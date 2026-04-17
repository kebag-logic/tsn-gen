# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

import json
import os
import subprocess
import tempfile

from behave import given, when, then


# ------------------------------------------------------------------ #
#  Given steps                                                        #
# ------------------------------------------------------------------ #

@given('the AECP protocol directory is loaded')
def step_aecp_dir(context):
    # yaml_dir already set in environment.py; just verify it's present.
    assert os.path.isdir(context.yaml_dir), \
        f"YAML dir not found: {context.yaml_dir}"


@given('the interface "{iface}"')
def step_set_iface(context, iface):
    context.current_iface = iface


# ------------------------------------------------------------------ #
#  When steps                                                         #
# ------------------------------------------------------------------ #

def _run_tool(context, count, seed=None, transport=None):
    cmd = [
        context.tool,
        "--yaml-dir", context.yaml_dir,
        "--interface", context.current_iface,
        "--count", str(count),
        "--output", "json",
    ]
    if seed is not None:
        cmd += ["--seed", str(seed)]
    if transport:
        cmd += ["--transport", transport]

    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0, \
        f"packet_gen failed (rc={result.returncode}):\n{result.stderr}"

    lines = [l for l in result.stdout.strip().splitlines() if l]
    assert len(lines) == count, \
        f"Expected {count} lines of output, got {len(lines)}"
    return [json.loads(l) for l in lines]


@when('I generate {count:d} packets with seed {seed:d}')
def step_gen_with_seed(context, count, seed):
    context.packets = _run_tool(context, count, seed=seed)


@when('I generate {count:d} packets')
def step_gen_random(context, count):
    context.packets = _run_tool(context, count)


@when('I generate {count:d} packets with seed {seed:d} to pcap file "{path}"')
def step_gen_to_pcap(context, count, seed, path):
    context.packets = _run_tool(context, count, seed=seed,
                                 transport=f"pcap:{path}")
    context.pcap_path = path


@when('I generate {count:d} packets with seed {seed:d} to raw interface "{ifname}"')
def step_gen_to_raw(context, count, seed, ifname):
    context.packets = _run_tool(context, count, seed=seed,
                                 transport=f"raw:{ifname}")


@when('I generate {count:d} packets with seed {seed:d} to verilator socket "{sock}"')
def step_gen_to_verilator(context, count, seed, sock):
    context.packets = _run_tool(context, count, seed=seed,
                                 transport=f"verilator:{sock}")


# ------------------------------------------------------------------ #
#  Then steps — field constraint assertions                           #
# ------------------------------------------------------------------ #

@then('the field "{field}" always equals {value:d}')
def step_field_equals(context, field, value):
    for i, pkt in enumerate(context.packets):
        actual = pkt["fields"].get(field)
        assert actual is not None, \
            f"packet {i}: field '{field}' not present in output"
        assert actual == value, \
            f"packet {i}: field '{field}' = {actual}, expected {value}"


@then('the field "{field}" always equals hex {value}')
def step_field_equals_hex(context, value, field):
    expected = int(value, 16)
    for i, pkt in enumerate(context.packets):
        actual = pkt["fields"].get(field)
        assert actual is not None, \
            f"packet {i}: field '{field}' not present"
        assert actual == expected, \
            f"packet {i}: field '{field}' = {actual:#x}, expected {expected:#x}"


@then('the field "{field}" is always in range [{lo:d}, {hi:d}]')
def step_field_in_range(context, field, lo, hi):
    for i, pkt in enumerate(context.packets):
        actual = pkt["fields"].get(field)
        assert actual is not None, \
            f"packet {i}: field '{field}' not present"
        assert lo <= actual <= hi, \
            f"packet {i}: field '{field}' = {actual}, not in [{lo}, {hi}]"


@then('the field "{field}" always satisfies mask {mask}')
def step_field_mask(context, field, mask):
    m = int(mask, 0)
    for i, pkt in enumerate(context.packets):
        actual = pkt["fields"].get(field)
        assert actual is not None, \
            f"packet {i}: field '{field}' not present"
        assert (actual & ~m) == 0, \
            f"packet {i}: field '{field}' = {actual:#x} has bits outside mask {m:#x}"


@then('the field "{field}" is always one of {values}')
def step_field_one_of(context, field, values):
    allowed = set(int(v.strip()) for v in values.strip("[]").split(","))
    for i, pkt in enumerate(context.packets):
        actual = pkt["fields"].get(field)
        assert actual is not None, \
            f"packet {i}: field '{field}' not present"
        assert actual in allowed, \
            f"packet {i}: field '{field}' = {actual}, not in {allowed}"


@then('the packet size is always {size:d} bytes')
def step_packet_size(context, size):
    for i, pkt in enumerate(context.packets):
        actual = len(bytes.fromhex(pkt["hex"]))
        assert actual == size, \
            f"packet {i}: size = {actual}, expected {size}"


# ------------------------------------------------------------------ #
#  Then steps — pcap output                                           #
# ------------------------------------------------------------------ #

@then('the pcap file "{path}" exists')
def step_pcap_exists(context, path):
    assert os.path.isfile(path), f"pcap file not found: {path}"


@then('the pcap file "{path}" contains {count:d} packets')
def step_pcap_count(context, path, count):
    """Parse the raw pcap format (no libpcap dependency)."""
    with open(path, "rb") as f:
        hdr = f.read(24)
    assert len(hdr) == 24, "pcap file too short (missing global header)"

    import struct
    magic = struct.unpack_from("<I", hdr)[0]
    assert magic in (0xA1B2C3D4, 0xD4C3B2A1), \
        f"Not a pcap file (magic={magic:#010x})"

    with open(path, "rb") as f:
        f.read(24)  # skip global header
        n = 0
        while True:
            ph = f.read(16)
            if len(ph) < 16:
                break
            incl_len = struct.unpack_from("<I", ph, 8)[0]
            f.read(incl_len)
            n += 1

    assert n == count, f"pcap contains {n} packets, expected {count}"
