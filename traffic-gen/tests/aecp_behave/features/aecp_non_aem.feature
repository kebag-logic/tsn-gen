# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for non-AEM AECP message types:
#   ADDRESS_ACCESS (message_type=2/3), VENDOR_UNIQUE (message_type=6/7).

Feature: AECP non-AEM PDU generation (ADDRESS_ACCESS and VENDOR_UNIQUE)

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  ADDRESS_ACCESS (message_type = 2 or 3)                           #
  # ------------------------------------------------------------------ #

  Scenario: ADDRESS_ACCESS message_type is COMMAND or RESPONSE
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 500 packets with seed 300
    Then the field "message_type" is always one of [2, 3]

  Scenario: ADDRESS_ACCESS control_data_length is always 38
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 100 packets with seed 301
    Then the field "control_data_length" always equals 38

  Scenario: ADDRESS_ACCESS tlv_count is always 1
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 200 packets with seed 302
    Then the field "tlv_count" always equals 1

  Scenario: ADDRESS_ACCESS tlv_length_and_mode respects defined bit mask
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 500 packets with seed 303
    Then the field "tlv_length_and_mode" always satisfies mask 0xFF0F

  Scenario: ADDRESS_ACCESS packet is 41 bytes (324 bits padded)
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 50 packets with seed 304
    Then the packet size is always 41 bytes

  Scenario: ADDRESS_ACCESS packets are captured to a pcap file
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 20 packets with seed 310 to pcap file "/tmp/aecp_addr_access.pcap"
    Then the pcap file "/tmp/aecp_addr_access.pcap" exists
    And the pcap file "/tmp/aecp_addr_access.pcap" contains 20 packets

  # ------------------------------------------------------------------ #
  #  VENDOR_UNIQUE (message_type = 6 or 7)                            #
  # ------------------------------------------------------------------ #

  Scenario: VENDOR_UNIQUE message_type is COMMAND or RESPONSE
    Given the interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When I generate 500 packets with seed 320
    Then the field "message_type" is always one of [6, 7]

  Scenario: VENDOR_UNIQUE control_data_length is always 32
    Given the interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When I generate 100 packets with seed 321
    Then the field "control_data_length" always equals 32

  Scenario: VENDOR_UNIQUE packet is 35 bytes (276 bits padded)
    Given the interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When I generate 50 packets with seed 322
    Then the packet size is always 35 bytes

  Scenario: VENDOR_UNIQUE packets are captured to a pcap file
    Given the interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When I generate 30 packets with seed 330 to pcap file "/tmp/aecp_vendor_unique.pcap"
    Then the pcap file "/tmp/aecp_vendor_unique.pcap" exists
    And the pcap file "/tmp/aecp_vendor_unique.pcap" contains 30 packets

  # ------------------------------------------------------------------ #
  #  Hardware-dependent transport scenarios (skipped in CI)            #
  # ------------------------------------------------------------------ #

  @hardware
  Scenario: ADDRESS_ACCESS packets are sent on a raw network interface
    Given the interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When I generate 5 packets with seed 340 to raw interface "lo"
    Then the field "message_type" is always one of [2, 3]

  @verilator
  Scenario: VENDOR_UNIQUE packets are sent to a Verilator DUT
    Given the interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When I generate 5 packets with seed 350 to verilator socket "/tmp/verilator.sock"
    Then the field "message_type" is always one of [6, 7]
