# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for audio-mapping AEM commands:
#   GET_AUDIO_MAP (cmd=43), ADD/REMOVE_AUDIO_MAPPINGS (cmd=44/45).

Feature: AECP AEM audio mapping PDU generation

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  GET_AUDIO_MAP (command_type = 43)                                 #
  # ------------------------------------------------------------------ #

  Scenario: GET_AUDIO_MAP command_type is always 43
    Given the interface "atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF"
    When I generate 200 packets with seed 150
    Then the field "command_type" always equals 43

  Scenario: GET_AUDIO_MAP control_data_length is always 28
    Given the interface "atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF"
    When I generate 100 packets with seed 151
    Then the field "control_data_length" always equals 28

  Scenario: GET_AUDIO_MAP reserved field is always zero
    Given the interface "atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF"
    When I generate 200 packets with seed 152
    Then the field "reserved" always equals 0

  Scenario: GET_AUDIO_MAP descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF"
    When I generate 500 packets with seed 153
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: GET_AUDIO_MAP packet is 31 bytes (244 bits padded)
    Given the interface "atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF"
    When I generate 50 packets with seed 154
    Then the packet size is always 31 bytes

  # ------------------------------------------------------------------ #
  #  ADD / REMOVE AUDIO MAPPINGS (command_type = 44 or 45)            #
  # ------------------------------------------------------------------ #

  Scenario: Audio mappings command_type is ADD or REMOVE
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 500 packets with seed 160
    Then the field "command_type" is always one of [44, 45]

  Scenario: Audio mappings number_of_mappings is always 1
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 200 packets with seed 161
    Then the field "number_of_mappings" always equals 1

  Scenario: Audio mappings control_data_length is always 36
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 100 packets with seed 162
    Then the field "control_data_length" always equals 36

  Scenario: Audio mappings reserved field is always zero
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 200 packets with seed 163
    Then the field "reserved" always equals 0

  Scenario: Audio mappings descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 500 packets with seed 164
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: Audio mappings packet is 39 bytes (308 bits padded)
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 50 packets with seed 165
    Then the packet size is always 39 bytes

  # ------------------------------------------------------------------ #
  #  Transport: audio mapping packets to pcap                          #
  # ------------------------------------------------------------------ #

  Scenario: ADD_AUDIO_MAPPINGS packets are captured to a pcap file
    Given the interface "atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF"
    When I generate 15 packets with seed 170 to pcap file "/tmp/aecp_audio_map.pcap"
    Then the pcap file "/tmp/aecp_audio_map.pcap" exists
    And the pcap file "/tmp/aecp_audio_map.pcap" contains 15 packets
