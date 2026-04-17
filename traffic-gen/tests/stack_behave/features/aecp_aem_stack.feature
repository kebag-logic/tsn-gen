# SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
# SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT
#
# Full on-wire frame tests for AECP AEM command/response interfaces.
#
# Every scenario stacks:
#   Ethernet MAC header  (14 B)  ethertype = 0x22F0
#   AVTP control header  ( 2 B)  subtype   = 0xFB
#   AECP AEM PDU         (variable)
#
# Expected frame sizes (measured):
#   AECP_AEM_NO_PAYLOAD        :  39 B  (20 B common + 2 B AEM header, 1 B pad)
#   AECP_SET_CONFIGURATION     :  43 B
#   AECP_AEM_DESC_QUERY        :  43 B
#   AECP_ABORT_OPERATION       :  47 B
#   AECP_GET_AUDIO_MAP         :  47 B
#   AECP_GET_NAME              :  47 B
#   AECP_OPERATION_STATUS      :  47 B
#   AECP_READ_DESCRIPTOR       :  47 B
#   AECP_SET_CLOCK_SOURCE      :  47 B
#   AECP_SET_SAMPLING_RATE     :  47 B
#   AECP_START_OPERATION       :  47 B
#   AECP_SET_CONTROL           :  51 B
#   AECP_SET_STREAM_FORMAT     :  51 B
#   AECP_ACQUIRE_ENTITY        :  55 B
#   AECP_AUDIO_MAPPINGS        :  55 B
#   AECP_LOCK_ENTITY           :  55 B
#   AECP_SET_STREAM_INFO       :  91 B
#   AECP_SET_NAME              : 111 B

Feature: AECP AEM — full on-wire frame assembly via layer stacking

  Background:
    Given the protocols root is loaded

  # ---------------------------------------------------------------- #
  #  Frame size                                                       #
  # ---------------------------------------------------------------- #

  Scenario Outline: <iface_short> stacked frame has the correct total size
    Given the application interface "<iface>"
    When  I generate 10 stacked packets with seed 1
    Then  the stacked packet size is always <size> bytes

    Examples: AECP AEM interfaces
      | iface_short            | iface                                                                                   | size |
      | AEM_NO_PAYLOAD         | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                 |   39 |
      | SET_CONFIGURATION      | atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF        |   43 |
      | AEM_DESC_QUERY         | atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF     |   43 |
      | ABORT_OPERATION        | atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF              |   47 |
      | GET_AUDIO_MAP          | atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF                    |   47 |
      | GET_NAME               | atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF                                   |   47 |
      | OPERATION_STATUS       | atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF           |   47 |
      | READ_DESCRIPTOR        | atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF              |   47 |
      | SET_CLOCK_SOURCE       | atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF           |   47 |
      | SET_SAMPLING_RATE      | atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF        |   47 |
      | START_OPERATION        | atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF              |   47 |
      | SET_CONTROL            | atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF                          |   51 |
      | SET_STREAM_FORMAT      | atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF        |   51 |
      | ACQUIRE_ENTITY         | atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF                 |   55 |
      | AUDIO_MAPPINGS         | atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF                 |   55 |
      | LOCK_ENTITY            | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                          |   55 |
      | SET_STREAM_INFO        | atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF              |   91 |
      | SET_NAME               | atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF                                   |  111 |

  # ---------------------------------------------------------------- #
  #  Ethernet MAC layer invariants                                    #
  # ---------------------------------------------------------------- #

  Scenario Outline: <iface_short> — ethertype is always 0x22F0
    Given the application interface "<iface>"
    When  I generate 20 stacked packets with seed 7
    Then  layer 0 field "ethertype" always equals hex 22F0

    Examples: AECP AEM interfaces
      | iface_short            | iface                                                                                   |
      | AEM_NO_PAYLOAD         | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                 |
      | SET_CONFIGURATION      | atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF        |
      | AEM_DESC_QUERY         | atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF     |
      | ABORT_OPERATION        | atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF              |
      | GET_AUDIO_MAP          | atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF                    |
      | GET_NAME               | atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF                                   |
      | OPERATION_STATUS       | atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF           |
      | READ_DESCRIPTOR        | atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF              |
      | SET_CLOCK_SOURCE       | atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF           |
      | SET_SAMPLING_RATE      | atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF        |
      | START_OPERATION        | atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF              |
      | SET_CONTROL            | atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF                          |
      | SET_STREAM_FORMAT      | atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF        |
      | ACQUIRE_ENTITY         | atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF                 |
      | AUDIO_MAPPINGS         | atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF                 |
      | LOCK_ENTITY            | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                          |
      | SET_STREAM_INFO        | atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF              |
      | SET_NAME               | atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF                                   |

  # ---------------------------------------------------------------- #
  #  AVTP layer invariants                                            #
  # ---------------------------------------------------------------- #

  Scenario Outline: <iface_short> — AVTP subtype is always 0xFB and version 0
    Given the application interface "<iface>"
    When  I generate 20 stacked packets with seed 3
    Then  layer 1 field "subtype" always equals 251
    And   layer 1 field "sv" always equals 0
    And   layer 1 field "version" always equals 0

    Examples: AECP AEM interfaces
      | iface_short            | iface                                                                                   |
      | AEM_NO_PAYLOAD         | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                 |
      | SET_CONFIGURATION      | atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF        |
      | AEM_DESC_QUERY         | atdecc_aecp_aem_descriptor_query::AECP_AEM_DESCRIPTOR_QUERY::AECP_AEM_DESC_QUERY_IF     |
      | ABORT_OPERATION        | atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF              |
      | GET_AUDIO_MAP          | atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF                    |
      | GET_NAME               | atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF                                   |
      | OPERATION_STATUS       | atdecc_aecp_operation_status::AECP_OPERATION_STATUS::AECP_OPERATION_STATUS_IF           |
      | READ_DESCRIPTOR        | atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF              |
      | SET_CLOCK_SOURCE       | atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF           |
      | SET_SAMPLING_RATE      | atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF        |
      | START_OPERATION        | atdecc_aecp_start_operation::AECP_START_OPERATION::AECP_START_OPERATION_IF              |
      | SET_CONTROL            | atdecc_aecp_set_control::AECP_SET_CONTROL::AECP_SET_CONTROL_IF                          |
      | SET_STREAM_FORMAT      | atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF        |
      | ACQUIRE_ENTITY         | atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF                 |
      | AUDIO_MAPPINGS         | atdecc_aecp_audio_mappings::AECP_AUDIO_MAPPINGS::AECP_AUDIO_MAPPINGS_IF                 |
      | LOCK_ENTITY            | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                          |
      | SET_STREAM_INFO        | atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF              |
      | SET_NAME               | atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF                                   |

  # ---------------------------------------------------------------- #
  #  AECP application-layer fixed-field invariants                   #
  # ---------------------------------------------------------------- #

  Scenario Outline: <iface_short> — control_data_length is always fixed
    Given the application interface "<iface>"
    When  I generate 20 stacked packets with seed 9
    Then  layer 2 field "control_data_length" always equals <cdl>

    Examples: AECP interfaces with fixed control_data_length
      | iface_short            | iface                                                                                   | cdl |
      | AEM_NO_PAYLOAD         | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                 |  20 |
      | SET_CONFIGURATION      | atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF        |  24 |
      | ABORT_OPERATION        | atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF              |  28 |
      | GET_AUDIO_MAP          | atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF                    |  28 |
      | READ_DESCRIPTOR        | atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF              |  28 |
      | SET_CLOCK_SOURCE       | atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF           |  28 |
      | SET_SAMPLING_RATE      | atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF        |  28 |
      | ACQUIRE_ENTITY         | atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF                 |  36 |
      | LOCK_ENTITY            | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                          |  36 |

  Scenario Outline: <iface_short> — message_type is always a command (0) or response (1)
    Given the application interface "<iface>"
    When  I generate 30 stacked packets with seed 11
    Then  layer 2 field "message_type" is always one of [0, 1]

    Examples: AECP AEM interfaces
      | iface_short            | iface                                                                                   |
      | AEM_NO_PAYLOAD         | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                 |
      | SET_CONFIGURATION      | atdecc_aecp_set_configuration::AECP_SET_CONFIGURATION::AECP_SET_CONFIGURATION_IF        |
      | ABORT_OPERATION        | atdecc_aecp_abort_operation::AECP_ABORT_OPERATION::AECP_ABORT_OPERATION_IF              |
      | GET_AUDIO_MAP          | atdecc_aecp_get_audio_map::AECP_GET_AUDIO_MAP::AECP_GET_AUDIO_MAP_IF                    |
      | GET_NAME               | atdecc_aecp_get_name::AECP_GET_NAME::AECP_GET_NAME_IF                                   |
      | READ_DESCRIPTOR        | atdecc_aecp_read_descriptor::AECP_READ_DESCRIPTOR::AECP_READ_DESCRIPTOR_IF              |
      | SET_CLOCK_SOURCE       | atdecc_aecp_set_clock_source::AECP_SET_CLOCK_SOURCE::AECP_SET_CLOCK_SOURCE_IF           |
      | SET_SAMPLING_RATE      | atdecc_aecp_set_sampling_rate::AECP_SET_SAMPLING_RATE::AECP_SET_SAMPLING_RATE_IF        |
      | ACQUIRE_ENTITY         | atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF                 |
      | LOCK_ENTITY            | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                          |
      | SET_STREAM_FORMAT      | atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF        |

  # ---------------------------------------------------------------- #
  #  pcap transport                                                   #
  # ---------------------------------------------------------------- #

  Scenario Outline: <iface_short> — stacked frame can be written to pcap
    Given the application interface "<iface>"
    When  I generate 5 stacked packets with seed 42 to pcap file "/tmp/stack_<iface_short>.pcap"
    Then  the pcap file "/tmp/stack_<iface_short>.pcap" exists
    And   the pcap file "/tmp/stack_<iface_short>.pcap" contains 5 packets

    Examples: AECP AEM interfaces
      | iface_short            | iface                                                                                   |
      | AEM_NO_PAYLOAD         | atdecc_aecp_aem_no_payload::AECP_AEM_NO_PAYLOAD::AECP_AEM_NO_PAYLOAD_IF                 |
      | ACQUIRE_ENTITY         | atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF                 |
      | LOCK_ENTITY            | atdecc_aecp_lock_entity::AECP_LOCK_ENTITY::AECP_LOCK_ENTITY_IF                          |
      | SET_STREAM_FORMAT      | atdecc_aecp_set_stream_format::AECP_SET_STREAM_FORMAT::AECP_SET_STREAM_FORMAT_IF        |
      | SET_STREAM_INFO        | atdecc_aecp_set_stream_info::AECP_SET_STREAM_INFO::AECP_SET_STREAM_INFO_IF              |
      | SET_NAME               | atdecc_aecp_set_name::AECP_SET_NAME::AECP_SET_NAME_IF                                   |
