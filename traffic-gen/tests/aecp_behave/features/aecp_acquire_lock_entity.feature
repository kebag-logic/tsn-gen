# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for AECP AEM ACQUIRE_ENTITY (cmd=0) and LOCK_ENTITY (cmd=1).

Feature: AECP AEM ACQUIRE_ENTITY and LOCK_ENTITY PDU generation

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  ACQUIRE_ENTITY (command_type = 0)                                 #
  # ------------------------------------------------------------------ #

  Scenario: ACQUIRE_ENTITY command_type is always 0
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 200 packets with seed 10
    Then the field "command_type" always equals 0

  Scenario: ACQUIRE_ENTITY acquire_entity_flags respects PERSISTENT|RELEASE mask
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 500 packets with seed 11
    Then the field "acquire_entity_flags" always satisfies mask 0xC0000000

  Scenario: ACQUIRE_ENTITY descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 500 packets with seed 12
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: ACQUIRE_ENTITY message_type is AEM_COMMAND or AEM_RESPONSE
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 500 packets with seed 13
    Then the field "message_type" is always one of [0, 1]

  Scenario: ACQUIRE_ENTITY control_data_length is always 36
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 100 packets with seed 14
    Then the field "control_data_length" always equals 36

  Scenario: ACQUIRE_ENTITY packet is 39 bytes (308 bits padded)
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 50 packets with seed 15
    Then the packet size is always 39 bytes

  # ------------------------------------------------------------------ #
  #  LOCK_ENTITY (command_type = 1)                                    #
  # ------------------------------------------------------------------ #

  Scenario: LOCK_ENTITY command_type is always 1
    Given the interface "atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF"
    When I generate 200 packets with seed 20
    Then the field "command_type" always equals 1

  Scenario: LOCK_ENTITY lock_entity_flags respects UNLOCK mask
    Given the interface "atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF"
    When I generate 500 packets with seed 21
    Then the field "lock_entity_flags" always satisfies mask 0x80000000

  Scenario: LOCK_ENTITY descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF"
    When I generate 500 packets with seed 22
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: LOCK_ENTITY control_data_length is always 36
    Given the interface "atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF"
    When I generate 100 packets with seed 23
    Then the field "control_data_length" always equals 36

  # ------------------------------------------------------------------ #
  #  Transport: write ACQUIRE_ENTITY packets to a pcap file            #
  # ------------------------------------------------------------------ #

  Scenario: ACQUIRE_ENTITY packets are written to a pcap capture file
    Given the interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"
    When I generate 10 packets with seed 30 to pcap file "/tmp/aecp_acquire.pcap"
    Then the pcap file "/tmp/aecp_acquire.pcap" exists
    And the pcap file "/tmp/aecp_acquire.pcap" contains 10 packets
