# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT
#
# Full on-wire frame tests for IEEE 1722.1 ADP (AVDECC Discovery Protocol).
#
# Only ADP_IF_DATA is tested here — it carries the single message_type field.
#
# Frame size:
#   Eth 14 B + AVTP 2 B + ADP_IF_DATA 1 B = 17 B

Feature: ADP — full on-wire frame assembly via layer stacking

  Background:
    Given the protocols root is loaded

  Scenario: ADP_IF_DATA stacked frame has correct total size
    Given the application interface "atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA"
    When  I generate 20 stacked packets with seed 1
    Then  the stacked packet size is always 17 bytes

  Scenario: ADP_IF_DATA — ethertype is always 0x22F0
    Given the application interface "atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA"
    When  I generate 30 stacked packets with seed 5
    Then  layer 0 field "ethertype" always equals hex 22F0

  Scenario: ADP_IF_DATA — AVTP subtype is always 0xFB and version 0
    Given the application interface "atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA"
    When  I generate 30 stacked packets with seed 3
    Then  layer 1 field "subtype" always equals 251
    And   layer 1 field "sv" always equals 0
    And   layer 1 field "version" always equals 0

  Scenario: ADP_IF_DATA — message_type is always 0 (ENTITY_AVAILABLE), 1 (ENTITY_DEPARTING), or 2 (ENTITY_DISCOVER)
    Given the application interface "atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA"
    When  I generate 50 stacked packets with seed 8
    Then  layer 2 field "message_type" is always one of [0, 1, 2]

  Scenario: ADP_IF_DATA — written to pcap
    Given the application interface "atdecc_adp_service::ADP_INTERFACE::ADP_IF_DATA"
    When  I generate 5 stacked packets with seed 42 to pcap file "/tmp/stack_ADP_IF_DATA.pcap"
    Then  the pcap file "/tmp/stack_ADP_IF_DATA.pcap" exists
    And   the pcap file "/tmp/stack_ADP_IF_DATA.pcap" contains 5 packets
