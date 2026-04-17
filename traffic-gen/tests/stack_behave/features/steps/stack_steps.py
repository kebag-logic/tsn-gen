# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

import json
import os
import subprocess

from behave import given, when, then

_ETH  = "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF"
_AVTP = "avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF"


def _make_stack(app_iface):
    return f"{_ETH}:{_AVTP}:{app_iface}"


def _run_stack(context, stack, count, seed=None, transport=None):
    cmd = [
        context.tool,
        "--yaml-dir", context.protocols_dir,
        "--stack", stack,
        "--count", str(count),
        "--output", "json",
    ]
    if seed is not None:
        cmd += ["--seed", str(seed)]
    if transport:
        cmd += ["--transport", transport]

    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0, \
        f"packet_gen --stack failed (rc={result.returncode}):\n{result.stderr}"

    lines = [l for l in result.stdout.strip().splitlines() if l.startswith("{")]
    assert len(lines) == count, \
        f"Expected {count} stacked packets, got {len(lines)}"
    return [json.loads(l) for l in lines]


# ------------------------------------------------------------------ #
#  Given                                                              #
# ------------------------------------------------------------------ #

@given('the protocols root is loaded')
def step_proto_root(context):
    assert os.path.isdir(context.protocols_dir), \
        f"Protocols root not found: {context.protocols_dir}"


@given('the application interface "{iface}"')
def step_set_app_iface(context, iface):
    context.app_iface = iface
    context.current_stack = _make_stack(iface)


# ------------------------------------------------------------------ #
#  When                                                               #
# ------------------------------------------------------------------ #

@when('I generate {count:d} stacked packets with seed {seed:d}')
def step_gen_stacked_seed(context, count, seed):
    context.packets = _run_stack(context, context.current_stack, count, seed=seed)


@when('I generate {count:d} stacked packets')
def step_gen_stacked_random(context, count):
    context.packets = _run_stack(context, context.current_stack, count)


@when('I generate {count:d} stacked packets with seed {seed:d} to pcap file "{path}"')
def step_gen_stacked_to_pcap(context, count, seed, path):
    context.packets = _run_stack(context, context.current_stack, count,
                                  seed=seed, transport=f"pcap:{path}")
    context.pcap_path = path


# ------------------------------------------------------------------ #
#  Then — frame-level assertions                                      #
# ------------------------------------------------------------------ #

@then('the stacked packet size is always {size:d} bytes')
def step_stacked_size(context, size):
    for i, pkt in enumerate(context.packets):
        actual = len(bytes.fromhex(pkt["hex"]))
        assert actual == size, \
            f"packet {i}: size = {actual}, expected {size}"


@then('layer {idx:d} field "{field}" always equals {value:d}')
def step_layer_field_equals(context, idx, field, value):
    for i, pkt in enumerate(context.packets):
        actual = pkt["layers"][idx]["fields"].get(field)
        assert actual is not None, \
            f"packet {i} layer {idx}: field '{field}' not present"
        assert actual == value, \
            f"packet {i} layer {idx}: '{field}' = {actual}, expected {value}"


@then('layer {idx:d} field "{field}" always equals hex {value}')
def step_layer_field_equals_hex(context, idx, field, value):
    expected = int(value, 16)
    for i, pkt in enumerate(context.packets):
        actual = pkt["layers"][idx]["fields"].get(field)
        assert actual is not None, \
            f"packet {i} layer {idx}: field '{field}' not present"
        assert actual == expected, \
            f"packet {i} layer {idx}: '{field}' = {actual:#x}, expected {expected:#x}"


@then('layer {idx:d} field "{field}" is always in range [{lo:d}, {hi:d}]')
def step_layer_field_in_range(context, idx, field, lo, hi):
    for i, pkt in enumerate(context.packets):
        actual = pkt["layers"][idx]["fields"].get(field)
        assert actual is not None, \
            f"packet {i} layer {idx}: field '{field}' not present"
        assert lo <= actual <= hi, \
            f"packet {i} layer {idx}: '{field}' = {actual}, not in [{lo}, {hi}]"


@then('layer {idx:d} field "{field}" is always one of {values}')
def step_layer_field_one_of(context, idx, field, values):
    allowed = set(int(v.strip()) for v in values.strip("[]").split(","))
    for i, pkt in enumerate(context.packets):
        actual = pkt["layers"][idx]["fields"].get(field)
        assert actual is not None, \
            f"packet {i} layer {idx}: field '{field}' not present"
        assert actual in allowed, \
            f"packet {i} layer {idx}: '{field}' = {actual}, not in {allowed}"


# ------------------------------------------------------------------ #
#  Then — pcap assertions                                             #
# ------------------------------------------------------------------ #

@then('the pcap file "{path}" exists')
def step_pcap_exists(context, path):
    assert os.path.isfile(path), f"pcap file not found: {path}"


@then('the pcap file "{path}" contains {count:d} packets')
def step_pcap_count(context, path, count):
    import struct
    with open(path, "rb") as f:
        hdr = f.read(24)
    assert len(hdr) == 24, "pcap file too short (missing global header)"
    magic = struct.unpack_from("<I", hdr)[0]
    assert magic in (0xA1B2C3D4, 0xD4C3B2A1), \
        f"Not a pcap file (magic={magic:#010x})"

    with open(path, "rb") as f:
        f.read(24)
        n = 0
        while True:
            ph = f.read(16)
            if len(ph) < 16:
                break
            incl_len = struct.unpack_from("<I", ph, 8)[0]
            f.read(incl_len)
            n += 1

    assert n == count, f"pcap contains {n} packets, expected {count}"


# ------------------------------------------------------------------ #
#  Decode helpers                                                     #
# ------------------------------------------------------------------ #

def _run_decode(context, stack=None, iface=None, transport=None, hex_input=None,
                count=None):
    cmd = [context.tool, "--yaml-dir", context.protocols_dir, "--decode"]
    if stack:
        cmd += ["--stack", stack]
    elif iface:
        cmd += ["--interface", iface]
    if hex_input:
        cmd += ["--hex", hex_input]
    if transport:
        cmd += ["--transport", transport]
        # Read until EOF; explicit count overrides
        cmd += ["--count", str(count) if count is not None else "0"]
    elif count is not None:
        cmd += ["--count", str(count)]
    cmd += ["--output", "json"]

    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0, \
        f"packet_gen --decode failed (rc={result.returncode}):\n{result.stderr}"

    lines = [l for l in result.stdout.strip().splitlines() if l.startswith("{")]
    return [json.loads(l) for l in lines]


# ------------------------------------------------------------------ #
#  When — decode                                                      #
# ------------------------------------------------------------------ #

@when('I decode the pcap file "{path}" against the stack')
def step_decode_pcap_stack(context, path):
    context.decoded = _run_decode(context, stack=context.current_stack,
                                   transport=f"pcap:{path}")


@when('I decode the pcap file "{path}" against interface "{iface}"')
def step_decode_pcap_iface(context, path, iface):
    context.decoded = _run_decode(context, iface=iface, transport=f"pcap:{path}")


@when('I decode hex "{hexstr}" against the stack')
def step_decode_hex_stack(context, hexstr):
    context.decoded = _run_decode(context, stack=context.current_stack,
                                   hex_input=hexstr)


@when('I decode hex "{hexstr}" against interface "{iface}"')
def step_decode_hex_iface(context, hexstr, iface):
    context.decoded = _run_decode(context, iface=iface, hex_input=hexstr)


# ------------------------------------------------------------------ #
#  Then — decoded assertions                                          #
# ------------------------------------------------------------------ #

@then('decoded layer {idx:d} field "{field}" always equals {value:d}')
def step_decoded_layer_field(context, idx, field, value):
    for i, pkt in enumerate(context.decoded):
        actual = pkt["layers"][idx]["fields"].get(field)
        assert actual is not None, \
            f"decoded packet {i} layer {idx}: field '{field}' not present"
        assert actual == value, \
            f"decoded packet {i} layer {idx}: '{field}' = {actual}, expected {value}"


@then('decoded layer {idx:d} field "{field}" always equals hex {value}')
def step_decoded_layer_field_hex(context, idx, field, value):
    expected = int(value, 16)
    for i, pkt in enumerate(context.decoded):
        actual = pkt["layers"][idx]["fields"].get(field)
        assert actual is not None, \
            f"decoded packet {i} layer {idx}: field '{field}' not present"
        assert actual == expected, \
            f"decoded packet {i} layer {idx}: '{field}' = {actual:#x}, expected {expected:#x}"


@then('the decoded packet count is {count:d}')
def step_decoded_count(context, count):
    assert len(context.decoded) == count, \
        f"decoded {len(context.decoded)} packets, expected {count}"


@then('decoded field "{field}" always equals {value:d}')
def step_decoded_field_single(context, field, value):
    for i, pkt in enumerate(context.decoded):
        actual = pkt["fields"].get(field)
        assert actual is not None, \
            f"decoded packet {i}: field '{field}' not present"
        assert actual == value, \
            f"decoded packet {i}: '{field}' = {actual}, expected {value}"


@then('the generated and decoded hex match')
def step_gen_decoded_hex_match(context):
    assert hasattr(context, 'packets') and hasattr(context, 'decoded'), \
        "Need both context.packets (generated) and context.decoded"
    assert len(context.packets) == len(context.decoded), \
        f"count mismatch: generated={len(context.packets)}, decoded={len(context.decoded)}"
    for i, (gen, dec) in enumerate(zip(context.packets, context.decoded)):
        assert gen["hex"] == dec["hex"], \
            f"packet {i}: generated hex differs from decoded hex\n" \
            f"  gen: {gen['hex']}\n  dec: {dec['hex']}"
