# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT
#
# Full on-wire frame tests for non-AEM AECP message types:
#   ADDRESS_ACCESS  (message_type 2/3)
#   VENDOR_UNIQUE   (message_type 6/7)
#
# Frame sizes:
#   AECP_ADDRESS_ACCESS  :  57 B  (Eth 14 + AVTP 2 + PDU 41)
#   AECP_VENDOR_UNIQUE   :  51 B  (Eth 14 + AVTP 2 + PDU 35)

Feature: AECP non-AEM — full on-wire frame assembly via layer stacking

  Background:
    Given the protocols root is loaded

  # ---------------------------------------------------------------- #
  #  ADDRESS_ACCESS                                                   #
  # ---------------------------------------------------------------- #

  Scenario: ADDRESS_ACCESS stacked frame has correct total size
    Given the application interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When  I generate 20 stacked packets with seed 1
    Then  the stacked packet size is always 57 bytes

  Scenario: ADDRESS_ACCESS — ethertype is always 0x22F0
    Given the application interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When  I generate 20 stacked packets with seed 5
    Then  layer 0 field "ethertype" always equals hex 22F0

  Scenario: ADDRESS_ACCESS — message_type is always 2 (command) or 3 (response)
    Given the application interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When  I generate 30 stacked packets with seed 13
    Then  layer 2 field "message_type" is always one of [2, 3]

  Scenario: ADDRESS_ACCESS — AVTP subtype and version are fixed
    Given the application interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When  I generate 20 stacked packets with seed 2
    Then  layer 1 field "subtype" always equals 251
    And   layer 1 field "sv" always equals 0
    And   layer 1 field "version" always equals 0

  Scenario: ADDRESS_ACCESS — written to pcap
    Given the application interface "atdecc_aecp_address_access::AECP_ADDRESS_ACCESS::AECP_ADDRESS_ACCESS_IF"
    When  I generate 5 stacked packets with seed 42 to pcap file "/tmp/stack_ADDRESS_ACCESS.pcap"
    Then  the pcap file "/tmp/stack_ADDRESS_ACCESS.pcap" exists
    And   the pcap file "/tmp/stack_ADDRESS_ACCESS.pcap" contains 5 packets

  # ---------------------------------------------------------------- #
  #  VENDOR_UNIQUE                                                    #
  # ---------------------------------------------------------------- #

  Scenario: VENDOR_UNIQUE stacked frame has correct total size
    Given the application interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When  I generate 20 stacked packets with seed 1
    Then  the stacked packet size is always 51 bytes

  Scenario: VENDOR_UNIQUE — ethertype is always 0x22F0
    Given the application interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When  I generate 20 stacked packets with seed 5
    Then  layer 0 field "ethertype" always equals hex 22F0

  Scenario: VENDOR_UNIQUE — message_type is always 6 (command) or 7 (response)
    Given the application interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When  I generate 30 stacked packets with seed 17
    Then  layer 2 field "message_type" is always one of [6, 7]

  Scenario: VENDOR_UNIQUE — AVTP subtype and version are fixed
    Given the application interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When  I generate 20 stacked packets with seed 2
    Then  layer 1 field "subtype" always equals 251
    And   layer 1 field "sv" always equals 0
    And   layer 1 field "version" always equals 0

  Scenario: VENDOR_UNIQUE — written to pcap
    Given the application interface "atdecc_aecp_vendor_unique::AECP_VENDOR_UNIQUE::AECP_VENDOR_UNIQUE_IF"
    When  I generate 5 stacked packets with seed 42 to pcap file "/tmp/stack_VENDOR_UNIQUE.pcap"
    Then  the pcap file "/tmp/stack_VENDOR_UNIQUE.pcap" exists
    And   the pcap file "/tmp/stack_VENDOR_UNIQUE.pcap" contains 5 packets
