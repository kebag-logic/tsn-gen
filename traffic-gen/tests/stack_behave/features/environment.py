# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

import os
import pathlib

# __file__ is features/environment.py
#   parents[0] = features/
#   parents[1] = stack_behave/
#   parents[2] = tests/
#   parents[3] = traffic-gen/
#   parents[4] = tsn-gen/ (repo root)
_REPO_ROOT = pathlib.Path(__file__).resolve().parents[4]

_DEFAULT_PROTOCOLS_ROOT = str(_REPO_ROOT / "protocols")
_DEFAULT_PACKET_GEN = str(_REPO_ROOT / "build" / "traffic-gen" / "packet_gen")


def before_all(context):
    context.protocols_dir = os.environ.get("PROTOCOLS_ROOT_DIR", _DEFAULT_PROTOCOLS_ROOT)
    context.tool = os.environ.get("PACKET_GEN_BIN", _DEFAULT_PACKET_GEN)

    if not os.path.isdir(context.protocols_dir):
        raise RuntimeError(
            f"Protocols root directory not found: {context.protocols_dir}\n"
            "Set PROTOCOLS_ROOT_DIR to the correct path."
        )
    if not os.path.isfile(context.tool):
        raise RuntimeError(
            f"packet_gen binary not found: {context.tool}\n"
            "Build the project first, or set PACKET_GEN_BIN."
        )
