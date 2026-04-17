# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

import os
import pathlib

# __file__ is inside features/ so parents[4] is the tsn-gen repo root.
#   parents[0] = features/
#   parents[1] = aecp_behave/
#   parents[2] = tests/
#   parents[3] = traffic-gen/
#   parents[4] = tsn-gen/ (repo root)
_REPO_ROOT = pathlib.Path(__file__).resolve().parents[4]

_DEFAULT_AECP_YAML_DIR = str(
    _REPO_ROOT / "protocols" / "application" / "1722_1" / "aecp"
)
_DEFAULT_PACKET_GEN = str(_REPO_ROOT / "build" / "traffic-gen" / "packet_gen")


def before_all(context):
    context.yaml_dir = os.environ.get("AECP_YAML_DIR", _DEFAULT_AECP_YAML_DIR)
    context.tool = os.environ.get("PACKET_GEN_BIN", _DEFAULT_PACKET_GEN)

    if not os.path.isdir(context.yaml_dir):
        raise RuntimeError(
            f"AECP YAML directory not found: {context.yaml_dir}\n"
            "Set AECP_YAML_DIR to the correct path."
        )
    if not os.path.isfile(context.tool):
        raise RuntimeError(
            f"packet_gen binary not found: {context.tool}\n"
            "Build the project first, or set PACKET_GEN_BIN."
        )
