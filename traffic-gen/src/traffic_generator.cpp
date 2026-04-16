/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <traffic_generator.h>

#include <iostream>

TrafficGenerator::TrafficGenerator(const ProtocolParser& parser,
                                   std::unique_ptr<ISender> sender)
    : mParser(parser),
      mSender(std::move(sender)),
      mBuilder(),
      mOpen(false)
{
}

const TrafficGeneratorErr TrafficGenerator::open()
{
    const SenderErr err = mSender->open();
    if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
        std::cerr << "TrafficGenerator: sender open failed\n";
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_NOT_OPEN);
    }
    mOpen = true;
    return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_SUCCESS);
}

void TrafficGenerator::close() noexcept
{
    if (mOpen) {
        mSender->close();
        mOpen = false;
    }
}

void TrafficGenerator::seedRng(uint64_t s)
{
    mBuilder.seed(s);
}

const TrafficGeneratorErr TrafficGenerator::sendOne(
    const ProtocolInterface& iface)
{
    const std::vector<uint8_t> pkt =
        mBuilder.build(iface, mParser.getVarDatabase());

    if (pkt.empty()) {
        std::cerr << "TrafficGenerator: empty packet for interface '"
                  << iface.getName() << "', skipping\n";
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_EMPTY_PACKET);
    }

    const SenderErr err = mSender->send(pkt);
    if (err.getErrorCode() != SenderErr::SENDER_SUCCESS) {
        std::cerr << "TrafficGenerator: send failed for interface '"
                  << iface.getName() << "'\n";
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_SEND_FAILED);
    }

    return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_SUCCESS);
}

const TrafficGeneratorErr TrafficGenerator::send(
    const std::string& qualifiedName)
{
    if (!mOpen) {
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_NOT_OPEN);
    }

    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(qualifiedName);
    if (iface == nullptr) {
        std::cerr << "TrafficGenerator: interface '" << qualifiedName
                  << "' not found\n";
        return TrafficGeneratorErr(
            TrafficGeneratorErr::TGEN_ERR_IFACE_NOT_FOUND);
    }

    return sendOne(*iface);
}

const TrafficGeneratorErr TrafficGenerator::sendFiltered(
    ProtocolInterface::IfDirections dir)
{
    if (!mOpen) {
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_NOT_OPEN);
    }

    const Database<ProtocolInterface>& db = mParser.getInterfaceDatabase();
    TrafficGeneratorErr lastErr(TrafficGeneratorErr::TGEN_SUCCESS);

    db.forEach([&](const std::string& /*name*/,
                   const ProtocolInterface& iface) {
        if (iface.getDirection() != dir) {
            return;
        }
        const TrafficGeneratorErr err = sendOne(iface);
        /* Record the last non-success error; keep going for other ifaces. */
        if (err.getErrorCode() != TrafficGeneratorErr::TGEN_SUCCESS &&
            err.getErrorCode() != TrafficGeneratorErr::TGEN_ERR_EMPTY_PACKET) {
            lastErr = err;
        }
    });

    return lastErr;
}

const TrafficGeneratorErr TrafficGenerator::sendAll()
{
    if (!mOpen) {
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_NOT_OPEN);
    }

    const Database<ProtocolInterface>& db = mParser.getInterfaceDatabase();
    TrafficGeneratorErr lastErr(TrafficGeneratorErr::TGEN_SUCCESS);

    db.forEach([&](const std::string& /*name*/,
                   const ProtocolInterface& iface) {
        const TrafficGeneratorErr err = sendOne(iface);
        if (err.getErrorCode() != TrafficGeneratorErr::TGEN_SUCCESS &&
            err.getErrorCode() != TrafficGeneratorErr::TGEN_ERR_EMPTY_PACKET) {
            lastErr = err;
        }
    });

    return lastErr;
}

const TrafficGeneratorErr TrafficGenerator::sendLoop(
    const std::string& qualifiedName, size_t count)
{
    if (!mOpen) {
        return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_ERR_NOT_OPEN);
    }

    const ProtocolInterface* iface =
        mParser.getInterfaceDatabase().getElement(qualifiedName);
    if (iface == nullptr) {
        return TrafficGeneratorErr(
            TrafficGeneratorErr::TGEN_ERR_IFACE_NOT_FOUND);
    }

    for (size_t i = 0; count == 0 || i < count; ++i) {
        const TrafficGeneratorErr err = sendOne(*iface);
        if (err.getErrorCode() == TrafficGeneratorErr::TGEN_ERR_SEND_FAILED) {
            return err;
        }
    }

    return TrafficGeneratorErr(TrafficGeneratorErr::TGEN_SUCCESS);
}
