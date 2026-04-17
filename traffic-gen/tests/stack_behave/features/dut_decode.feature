# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

Feature: Packet decode — DUT output verification

  Verify that PacketDecoder correctly reconstructs field values from raw byte
  buffers, and that the generate → capture → decode round-trip is lossless.

  Background:
    Given the protocols root is loaded
    And   the application interface "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF"

  # ------------------------------------------------------------------
  # Round-trip: generate to pcap, decode from pcap, verify hex matches
  # ------------------------------------------------------------------

  Scenario: Generate-decode round-trip via pcap preserves hex (ACQUIRE_ENTITY)
    When  I generate 5 stacked packets with seed 1 to pcap file "/tmp/rt_acquire.pcap"
    And   I decode the pcap file "/tmp/rt_acquire.pcap" against the stack
    Then  the decoded packet count is 5
    And   the generated and decoded hex match

  Scenario: Decoded Ethernet ethertype round-trips correctly
    When  I generate 10 stacked packets with seed 7 to pcap file "/tmp/rt_eth.pcap"
    And   I decode the pcap file "/tmp/rt_eth.pcap" against the stack
    Then  decoded layer 0 field "ethertype" always equals hex 22F0

  Scenario: Decoded AVTP subtype round-trips correctly
    When  I generate 10 stacked packets with seed 13 to pcap file "/tmp/rt_avtp.pcap"
    And   I decode the pcap file "/tmp/rt_avtp.pcap" against the stack
    Then  decoded layer 1 field "subtype" always equals 251

  # ------------------------------------------------------------------
  # Direct hex decode (no file I/O)
  # ------------------------------------------------------------------

  Scenario: Decode a fixed hex string — ethertype field is 0x22F0
    When  I decode hex "2245bd5fbb6822eb9250231822f0fb000382416e6678d39feef008e61bd8674b6331bca260000000000004ab7a473d29376c1001e797e0" against the stack
    Then  decoded layer 0 field "ethertype" always equals hex 22F0

  # ------------------------------------------------------------------
  # Round-trip across multiple interfaces
  # ------------------------------------------------------------------

  Scenario Outline: Round-trip preserves frame size for all AEM PDUs
    Given the application interface "<app_iface>"
    When  I generate 3 stacked packets with seed 42 to pcap file "/tmp/rt_<tag>.pcap"
    And   I decode the pcap file "/tmp/rt_<tag>.pcap" against the stack
    Then  the decoded packet count is 3
    And   the generated and decoded hex match

    Examples:
      | tag          | app_iface                                                                                         |
      | no_payload   | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                          |
      | set_config   | atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF                  |
      | lock         | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                                    |
      | read_desc    | atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF                        |
      | set_name     | atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF                                            |
      | get_name     | atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF                                            |
      | set_srate    | atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF                  |
      | set_format   | atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF                  |
      | set_info     | atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF                        |
      | set_clock    | atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF                    |
      | set_control  | atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF                                    |
      | start_op     | atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF                        |
      | abort_op     | atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF                        |
      | op_status    | atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF                    |
      | get_audio    | atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF                              |
      | audio_maps   | atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF                          |
