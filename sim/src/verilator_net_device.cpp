/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <verilator_net_device.h>

#include <ns3/log.h>
#include <ns3/simulator.h>

NS_LOG_COMPONENT_DEFINE("VerilatorNetDevice");

ns3::TypeId VerilatorNetDevice::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("VerilatorNetDevice")
        .SetParent<ns3::NetDevice>()
        .SetGroupName("tsn-sim")
        .AddConstructor<VerilatorNetDevice>();
    return tid;
}

VerilatorNetDevice::VerilatorNetDevice(AxisBridge* bridge)
    : m_bridge(bridge)
{
    NS_ASSERT(bridge != nullptr);
}

void VerilatorNetDevice::SetupRx()
{
    m_bridge->on_rx = [this](std::vector<uint8_t> bytes) {
        /* Called from SystemC rx_thread — schedule delivery into ns-3. */
        ns3::Simulator::ScheduleNow(&VerilatorNetDevice::DeliverPacket,
                                    this, std::move(bytes));
    };
}

void VerilatorNetDevice::DeliverPacket(std::vector<uint8_t> bytes)
{
    if (m_rxCallback.IsNull()) return;

    auto pkt = ns3::Create<ns3::Packet>(bytes.data(),
                                        static_cast<uint32_t>(bytes.size()));
    m_rxCallback(this, pkt, 0 /* ETH_P_ALL */, GetBroadcast());
}

/* ---------------------------------------------------------------------- */

bool VerilatorNetDevice::Send(ns3::Ptr<ns3::Packet> packet,
                              const ns3::Address&   /*dest*/,
                              uint16_t              /*protocolNumber*/)
{
    const uint32_t sz = packet->GetSize();
    std::vector<uint8_t> buf(sz);
    packet->CopyData(buf.data(), sz);

    if (!m_bridge->push_tx_packet(std::move(buf))) {
        NS_LOG_WARN("VerilatorNetDevice: TX FIFO full, packet dropped");
        return false;
    }
    return true;
}

bool VerilatorNetDevice::SendFrom(ns3::Ptr<ns3::Packet> packet,
                                  const ns3::Address&   source,
                                  const ns3::Address&   dest,
                                  uint16_t              protocolNumber)
{
    return Send(packet, dest, protocolNumber);
}

/* ---------------------------------------------------------------------- */
/* Boilerplate NetDevice accessors                                         */
/* ---------------------------------------------------------------------- */

void VerilatorNetDevice::SetIfIndex(uint32_t index) { m_ifIndex = index; }
uint32_t VerilatorNetDevice::GetIfIndex() const     { return m_ifIndex; }

ns3::Ptr<ns3::Channel> VerilatorNetDevice::GetChannel() const { return nullptr; }

void VerilatorNetDevice::SetAddress(ns3::Address address)
{
    m_address = ns3::Mac48Address::ConvertFrom(address);
}

ns3::Address VerilatorNetDevice::GetAddress() const { return m_address; }

bool     VerilatorNetDevice::SetMtu(uint16_t mtu) { m_mtu = mtu; return true; }
uint16_t VerilatorNetDevice::GetMtu()       const { return m_mtu; }

bool VerilatorNetDevice::IsLinkUp()  const { return true; }
void VerilatorNetDevice::AddLinkChangeCallback(ns3::Callback<void>) {}

bool         VerilatorNetDevice::IsBroadcast()   const { return true; }
ns3::Address VerilatorNetDevice::GetBroadcast()  const
{
    return ns3::Mac48Address::GetBroadcast();
}

bool VerilatorNetDevice::IsMulticast() const { return false; }
ns3::Address VerilatorNetDevice::GetMulticast(ns3::Ipv4Address) const
{
    return ns3::Mac48Address::GetMulticast(ns3::Ipv4Address());
}
ns3::Address VerilatorNetDevice::GetMulticast(ns3::Ipv6Address) const
{
    return ns3::Mac48Address::GetMulticast6(ns3::Ipv6Address());
}

bool VerilatorNetDevice::IsBridge()       const { return false; }
bool VerilatorNetDevice::IsPointToPoint() const { return true; }
bool VerilatorNetDevice::SupportsSendFrom() const { return true; }
bool VerilatorNetDevice::NeedsArp()       const { return false; }

ns3::Ptr<ns3::Node> VerilatorNetDevice::GetNode() const  { return m_node; }
void VerilatorNetDevice::SetNode(ns3::Ptr<ns3::Node> node) { m_node = node; }

void VerilatorNetDevice::SetReceiveCallback(ReceiveCallback cb)
{
    m_rxCallback = cb;
}

void VerilatorNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
    m_promiscRxCallback = cb;
}
