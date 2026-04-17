# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for name and control AEM commands:
#   SET_NAME (cmd=16), GET_NAME (cmd=17),
#   SET_SAMPLING_RATE (cmd=20), SET_CLOCK_SOURCE (cmd=22), SET_CONTROL (cmd=24).

Feature: AECP AEM name and control PDU generation

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  SET_NAME (command_type = 16)                                      #
  # ------------------------------------------------------------------ #

  Scenario: SET_NAME command_type is always 16
    Given the interface "atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF"
    When I generate 200 packets with seed 100
    Then the field "command_type" always equals 16

  Scenario: SET_NAME control_data_length is always 92
    Given the interface "atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF"
    When I generate 100 packets with seed 101
    Then the field "control_data_length" always equals 92

  Scenario: SET_NAME descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF"
    When I generate 500 packets with seed 102
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: SET_NAME packet is 95 bytes (756 bits padded)
    Given the interface "atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF"
    When I generate 50 packets with seed 103
    Then the packet size is always 95 bytes

  # ------------------------------------------------------------------ #
  #  GET_NAME (command_type = 17)                                      #
  # ------------------------------------------------------------------ #

  Scenario: GET_NAME command_type is always 17
    Given the interface "atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF"
    When I generate 200 packets with seed 110
    Then the field "command_type" always equals 17

  Scenario: GET_NAME control_data_length is always 28
    Given the interface "atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF"
    When I generate 100 packets with seed 111
    Then the field "control_data_length" always equals 28

  Scenario: GET_NAME packet is 31 bytes (244 bits padded)
    Given the interface "atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF"
    When I generate 50 packets with seed 112
    Then the packet size is always 31 bytes

  # ------------------------------------------------------------------ #
  #  SET_SAMPLING_RATE (command_type = 20)                             #
  # ------------------------------------------------------------------ #

  Scenario: SET_SAMPLING_RATE command_type is always 20
    Given the interface "atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF"
    When I generate 200 packets with seed 120
    Then the field "command_type" always equals 20

  Scenario: SET_SAMPLING_RATE control_data_length is always 28
    Given the interface "atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF"
    When I generate 100 packets with seed 121
    Then the field "control_data_length" always equals 28

  Scenario: SET_SAMPLING_RATE descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF"
    When I generate 500 packets with seed 122
    Then the field "descriptor_type" is always in range [0, 37]

  # ------------------------------------------------------------------ #
  #  SET_CLOCK_SOURCE (command_type = 22)                              #
  # ------------------------------------------------------------------ #

  Scenario: SET_CLOCK_SOURCE command_type is always 22
    Given the interface "atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF"
    When I generate 200 packets with seed 130
    Then the field "command_type" always equals 22

  Scenario: SET_CLOCK_SOURCE control_data_length is always 28
    Given the interface "atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF"
    When I generate 100 packets with seed 131
    Then the field "control_data_length" always equals 28

  Scenario: SET_CLOCK_SOURCE reserved field is always zero
    Given the interface "atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF"
    When I generate 200 packets with seed 132
    Then the field "reserved" always equals 0

  # ------------------------------------------------------------------ #
  #  SET_CONTROL (command_type = 24)                                   #
  # ------------------------------------------------------------------ #

  Scenario: SET_CONTROL command_type is always 24
    Given the interface "atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF"
    When I generate 200 packets with seed 140
    Then the field "command_type" always equals 24

  Scenario: SET_CONTROL control_data_length is always 32
    Given the interface "atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF"
    When I generate 100 packets with seed 141
    Then the field "control_data_length" always equals 32

  Scenario: SET_CONTROL descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF"
    When I generate 500 packets with seed 142
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: SET_CONTROL packet is 35 bytes (276 bits padded)
    Given the interface "atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF"
    When I generate 50 packets with seed 143
    Then the packet size is always 35 bytes
