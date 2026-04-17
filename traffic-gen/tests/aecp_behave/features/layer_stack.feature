# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT
#
# Tests for the --stack feature: byte-level layer concatenation.
# Verifies that Ethernet MAC + AVTP control + AECP PDU assemble into a
# complete on-wire frame with the correct size and fixed field values.
#
# On-wire layout:
#   Ethernet MAC header  :  14 bytes  (dst_mac + src_mac + ethertype=0x22F0)
#   AVTP control header  :   2 bytes  (subtype=0xFB + flags byte)
#   AECP ACQUIRE_ENTITY  :  39 bytes
#   ─────────────────────────────────
#   Total                :  55 bytes

Feature: Layer stacking — full on-wire frame assembly

  Background:
    Given the AECP protocol directory is loaded

  Scenario: Ethernet + AVTP + AECP_ACQUIRE_ENTITY stack produces 55-byte frame
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 20 stacked packets with seed 1
    Then  the stacked packet size is always 55 bytes

  Scenario: Ethernet MAC ethertype is always 0x22F0
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 50 stacked packets with seed 99
    Then  layer 0 field "ethertype" always equals 8944

  Scenario: Ethernet MAC ethertype is always 0x22F0 (hex check)
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 20 stacked packets with seed 7
    Then  layer 0 field "ethertype" always equals hex 22F0

  Scenario: AVTP subtype is always 0xFB (251 decimal)
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 20 stacked packets with seed 12
    Then  layer 1 field "subtype" always equals 251

  Scenario: AVTP sv and version are always zero
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 30 stacked packets with seed 5
    Then  layer 1 field "sv" always equals 0
    And   layer 1 field "version" always equals 0

  Scenario: AECP command_type is always ACQUIRE_ENTITY (0)
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 30 stacked packets with seed 77
    Then  layer 2 field "command_type" always equals 0

  Scenario: Stack output is deterministic with a fixed seed
    Given the interface stack "ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF:avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF:atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When  I generate 5 stacked packets with seed 42
    Then  the stacked packet size is always 55 bytes
    And   layer 0 field "ethertype" always equals 8944
    And   layer 1 field "subtype" always equals 251
