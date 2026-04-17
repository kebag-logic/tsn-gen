# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-License-Identifier: MIT
#
# Behaviour tests for descriptor-related AEM commands:
#   READ_DESCRIPTOR (cmd=4), SET_CONFIGURATION (cmd=6),
#   and single-descriptor query commands (9, 15, 21, 23, 25-27, 34-35, 39-42).

Feature: AECP AEM descriptor operation PDU generation

  Background:
    Given the AECP protocol directory is loaded

  # ------------------------------------------------------------------ #
  #  READ_DESCRIPTOR (command_type = 4)                                #
  # ------------------------------------------------------------------ #

  Scenario: READ_DESCRIPTOR command_type is always 4
    Given the interface "atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF"
    When I generate 200 packets with seed 40
    Then the field "command_type" always equals 4

  Scenario: READ_DESCRIPTOR control_data_length is always 28
    Given the interface "atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF"
    When I generate 100 packets with seed 41
    Then the field "control_data_length" always equals 28

  Scenario: READ_DESCRIPTOR reserved field is always zero
    Given the interface "atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF"
    When I generate 200 packets with seed 42
    Then the field "reserved" always equals 0

  Scenario: READ_DESCRIPTOR descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF"
    When I generate 500 packets with seed 43
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: READ_DESCRIPTOR packet is 31 bytes (244 bits padded)
    Given the interface "atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF"
    When I generate 50 packets with seed 44
    Then the packet size is always 31 bytes

  # ------------------------------------------------------------------ #
  #  SET_CONFIGURATION (command_type = 6)                              #
  # ------------------------------------------------------------------ #

  Scenario: SET_CONFIGURATION command_type is always 6
    Given the interface "atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF"
    When I generate 200 packets with seed 50
    Then the field "command_type" always equals 6

  Scenario: SET_CONFIGURATION control_data_length is always 24
    Given the interface "atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF"
    When I generate 100 packets with seed 51
    Then the field "control_data_length" always equals 24

  Scenario: SET_CONFIGURATION reserved field is always zero
    Given the interface "atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF"
    When I generate 200 packets with seed 52
    Then the field "reserved" always equals 0

  Scenario: SET_CONFIGURATION packet is 27 bytes (212 bits padded)
    Given the interface "atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF"
    When I generate 50 packets with seed 53
    Then the packet size is always 27 bytes

  # ------------------------------------------------------------------ #
  #  Descriptor query commands (CDL=24, 2 payload fields)              #
  # ------------------------------------------------------------------ #

  Scenario: Descriptor query command_type is one of the defined query commands
    Given the interface "atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF"
    When I generate 500 packets with seed 60
    Then the field "command_type" is always one of [9, 15, 21, 23, 25, 26, 27, 34, 35, 39, 40, 41, 42]

  Scenario: Descriptor query control_data_length is always 24
    Given the interface "atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF"
    When I generate 100 packets with seed 61
    Then the field "control_data_length" always equals 24

  Scenario: Descriptor query descriptor_type is a valid IEEE 1722.1 descriptor
    Given the interface "atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF"
    When I generate 500 packets with seed 62
    Then the field "descriptor_type" is always in range [0, 37]

  Scenario: Descriptor query packet is 27 bytes (212 bits padded)
    Given the interface "atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF"
    When I generate 50 packets with seed 63
    Then the packet size is always 27 bytes
