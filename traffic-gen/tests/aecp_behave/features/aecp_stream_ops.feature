# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for stream-related AEM commands:
#   SET_STREAM_FORMAT (cmd=8), SET_STREAM_INFO (cmd=14).

Feature: AECP AEM stream operation PDU generation

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  SET_STREAM_FORMAT (command_type = 8)                              #
  # ------------------------------------------------------------------ #

  Scenario: SET_STREAM_FORMAT command_type is always 8
    Given the interface "atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF"
    When I generate 200 packets with seed 70
    Then the field "command_type" always equals 8

  Scenario: SET_STREAM_FORMAT control_data_length is always 32
    Given the interface "atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF"
    When I generate 100 packets with seed 71
    Then the field "control_data_length" always equals 32

  Scenario: SET_STREAM_FORMAT descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF"
    When I generate 500 packets with seed 72
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: SET_STREAM_FORMAT packet is 35 bytes (276 bits padded)
    Given the interface "atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF"
    When I generate 50 packets with seed 73
    Then the packet size is always 35 bytes

  # ------------------------------------------------------------------ #
  #  SET_STREAM_INFO (command_type = 14)                               #
  # ------------------------------------------------------------------ #

  Scenario: SET_STREAM_INFO command_type is always 14
    Given the interface "atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF"
    When I generate 200 packets with seed 80
    Then the field "command_type" always equals 14

  Scenario: SET_STREAM_INFO control_data_length is always 68
    Given the interface "atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF"
    When I generate 100 packets with seed 81
    Then the field "control_data_length" always equals 68

  Scenario: SET_STREAM_INFO descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF"
    When I generate 500 packets with seed 82
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: SET_STREAM_INFO reserved fields are always zero
    Given the interface "atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF"
    When I generate 200 packets with seed 83
    Then the field "reserved0" always equals 0
    And the field "reserved1" always equals 0

  Scenario: SET_STREAM_INFO msrp_flags respects defined bit mask
    Given the interface "atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF"
    When I generate 500 packets with seed 84
    Then the field "msrp_flags" always satisfies mask 0xF800000000000000

  Scenario: SET_STREAM_INFO packet is 75 bytes (596 bits padded)
    Given the interface "atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF"
    When I generate 50 packets with seed 85
    Then the packet size is always 75 bytes

  # ------------------------------------------------------------------ #
  #  Transport: SET_STREAM_FORMAT packets to pcap                      #
  # ------------------------------------------------------------------ #

  Scenario: SET_STREAM_FORMAT packets are captured to a pcap file
    Given the interface "atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF"
    When I generate 20 packets with seed 90 to pcap file "/tmp/aecp_stream_format.pcap"
    Then the pcap file "/tmp/aecp_stream_format.pcap" exists
    And the pcap file "/tmp/aecp_stream_format.pcap" contains 20 packets
