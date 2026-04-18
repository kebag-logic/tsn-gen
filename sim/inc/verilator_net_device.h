/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <axis_bridge.h>

#include <ns3/net-device.h>
#include <ns3/mac48-address.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>

/**
 * @brief ns-3 NetDevice whose data plane is an AxisBridge SystemC module.
 *
 * ## TX path
 *   Send() serialises the ns-3 Packet to a raw byte vector and calls
 *   AxisBridge::push_tx_packet().  This is synchronous from ns-3's perspective;
 *   the AXI-Stream handshake with the DUT happens inside the SystemC kernel.
 *
 * ## RX path
 *   The device registers an on_rx callback with AxisBridge during SetupRx().
 *   When the DUT emits a complete packet, the callback is invoked from the
 *   SystemC rx_thread context.  It uses Simulator::ScheduleNow() to re-enter
 *   ns-3 and deliver the frame to the registered ReceiveCallback.
 *
 * ## Requirements
 *   ns-3 must use the SystemC-backed SimulatorImpl so that ScheduleNow() is
 *   safe to call from a SystemC SC_THREAD (single shared virtual clock).
 */
class VerilatorNetDevice : public ns3::NetDevice {
public:
    static ns3::TypeId GetTypeId();

    explicit VerilatorNetDevice(AxisBridge* bridge);

    /**
     * @brief Register the on_rx callback with the bridge.  Call once, before
     *        sc_start().
     */
    void SetupRx();

    /* ------------------------------------------------------------------
     * ns3::NetDevice interface
     * ------------------------------------------------------------------ */
    void     SetIfIndex(uint32_t index) override;
    uint32_t GetIfIndex() const override;

    ns3::Ptr<ns3::Channel> GetChannel() const override;

    void         SetAddress(ns3::Address address) override;
    ns3::Address GetAddress() const override;

    bool     SetMtu(uint16_t mtu) override;
    uint16_t GetMtu() const override;

    bool IsLinkUp() const override;
    void AddLinkChangeCallback(ns3::Callback<void> callback) override;

    bool         IsBroadcast() const override;
    ns3::Address GetBroadcast() const override;

    bool         IsMulticast() const override;
    ns3::Address GetMulticast(ns3::Ipv4Address group) const override;
    ns3::Address GetMulticast(ns3::Ipv6Address  addr)  const override;

    bool IsBridge()       const override;
    bool IsPointToPoint() const override;
    bool SupportsSendFrom() const override;
    bool NeedsArp() const override;

    bool Send(ns3::Ptr<ns3::Packet>  packet,
              const ns3::Address&    dest,
              uint16_t               protocolNumber) override;

    bool SendFrom(ns3::Ptr<ns3::Packet> packet,
                  const ns3::Address&   source,
                  const ns3::Address&   dest,
                  uint16_t              protocolNumber) override;

    ns3::Ptr<ns3::Node> GetNode() const override;
    void                SetNode(ns3::Ptr<ns3::Node> node) override;

    void SetReceiveCallback(ReceiveCallback cb) override;
    void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;

private:
    void DeliverPacket(std::vector<uint8_t> bytes);

    AxisBridge*            m_bridge;
    uint32_t               m_ifIndex{0};
    uint16_t               m_mtu{1500};
    ns3::Mac48Address      m_address;
    ns3::Ptr<ns3::Node>    m_node;
    ReceiveCallback        m_rxCallback;
    PromiscReceiveCallback m_promiscRxCallback;
};
