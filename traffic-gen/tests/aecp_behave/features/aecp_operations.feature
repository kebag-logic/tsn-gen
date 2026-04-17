# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for AECP AEM operation commands:
#   START_OPERATION (cmd=52), ABORT_OPERATION (cmd=53),
#   OPERATION_STATUS (cmd=54), and no-payload commands.

Feature: AECP AEM operation and no-payload PDU generation

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  START_OPERATION (command_type = 52)                               #
  # ------------------------------------------------------------------ #

  Scenario: START_OPERATION command_type is always 52
    Given the interface "atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF"
    When I generate 200 packets with seed 200
    Then the field "command_type" always equals 52

  Scenario: START_OPERATION control_data_length is always 28
    Given the interface "atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF"
    When I generate 100 packets with seed 201
    Then the field "control_data_length" always equals 28

  Scenario: START_OPERATION descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF"
    When I generate 500 packets with seed 202
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: START_OPERATION packet is 31 bytes (244 bits padded)
    Given the interface "atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF"
    When I generate 50 packets with seed 203
    Then the packet size is always 31 bytes

  # ------------------------------------------------------------------ #
  #  ABORT_OPERATION (command_type = 53)                               #
  # ------------------------------------------------------------------ #

  Scenario: ABORT_OPERATION command_type is always 53
    Given the interface "atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF"
    When I generate 200 packets with seed 210
    Then the field "command_type" always equals 53

  Scenario: ABORT_OPERATION control_data_length is always 28
    Given the interface "atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF"
    When I generate 100 packets with seed 211
    Then the field "control_data_length" always equals 28

  Scenario: ABORT_OPERATION reserved field is always zero
    Given the interface "atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF"
    When I generate 200 packets with seed 212
    Then the field "reserved" always equals 0

  # ------------------------------------------------------------------ #
  #  OPERATION_STATUS (command_type = 54)                              #
  # ------------------------------------------------------------------ #

  Scenario: OPERATION_STATUS command_type is always 54
    Given the interface "atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF"
    When I generate 200 packets with seed 220
    Then the field "command_type" always equals 54

  Scenario: OPERATION_STATUS percent_complete is always in [0, 100]
    Given the interface "atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF"
    When I generate 500 packets with seed 221
    Then the field "percent_complete" is always in range [0, 100]

  Scenario: OPERATION_STATUS control_data_length is always 28
    Given the interface "atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF"
    When I generate 100 packets with seed 222
    Then the field "control_data_length" always equals 28

  # ------------------------------------------------------------------ #
  #  No-payload commands (CDL=20)                                      #
  # ------------------------------------------------------------------ #

  Scenario: No-payload command_type is one of the defined no-payload commands
    Given the interface "atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF"
    When I generate 500 packets with seed 230
    Then the field "command_type" is always one of [2, 3, 7, 36, 37]

  Scenario: No-payload control_data_length is always 20
    Given the interface "atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF"
    When I generate 100 packets with seed 231
    Then the field "control_data_length" always equals 20

  Scenario: No-payload packet is 23 bytes (180 bits padded)
    Given the interface "atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF"
    When I generate 50 packets with seed 232
    Then the packet size is always 23 bytes

  # ------------------------------------------------------------------ #
  #  Transport: OPERATION_STATUS to pcap                               #
  # ------------------------------------------------------------------ #

  Scenario: OPERATION_STATUS packets are captured to a pcap file
    Given the interface "atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF"
    When I generate 25 packets with seed 240 to pcap file "/tmp/aecp_op_status.pcap"
    Then the pcap file "/tmp/aecp_op_status.pcap" exists
    And the pcap file "/tmp/aecp_op_status.pcap" contains 25 packets
