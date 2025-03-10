Protocols
=========

In this section the protocols are executed providing a direct introspection
Using a clear separation between the protocols and the frames provides
flexibility for testing, understanding and investigating.

# Interfaces

The interface must be generic for each of the protocol involved. 
It does not aims to monitor internal signal of system but only a selected set
of variables defined in protocols

# OSI Layers

Sure! Here's a table with at least **five examples** for each **OSI model layer**:

| **OSI Layer**           | **Function**                                       | **Examples** (at least 5) |
|-------------------------|---------------------------------------------------|---------------------------|
| **Layer 1 – Physical**  | Hardware transmission, signals, and media        | Ethernet (IEEE 802.3), Wi-Fi (IEEE 802.11), Fiber Optic Cables, Coaxial Cables, Bluetooth (IEEE 802.15.1) |
| **Layer 2 – Data Link** | MAC addressing, frame transmission, error detection | Ethernet (IEEE 802.3 MAC), Wi-Fi (IEEE 802.11 MAC), VLAN (IEEE 802.1Q), AVB/TSN (IEEE 802.1AS, 802.1Qat, 802.1Qav), LLDP (IEEE 802.1AB) |
| **Layer 3 – Network**   | IP addressing, routing, and packet forwarding    | IPv4, IPv6, ICMP (ping), OSPF (Open Shortest Path First), BGP (Border Gateway Protocol) |
| **Layer 4 – Transport** | End-to-end communication, reliability, and flow control | TCP (Transmission Control Protocol), UDP (User Datagram Protocol), SCTP (Stream Control Transmission Protocol), DCCP (Datagram Congestion Control Protocol), QUIC (Quick UDP Internet Connections) |
| **Layer 5 – Session**   | Manages sessions, maintains connections          | TLS (Transport Layer Security), NetBIOS, SIP (Session Initiation Protocol), RPC (Remote Procedure Call), SOCKS (Socket Secure) |
| **Layer 6 – Presentation** | Data encoding, encryption, and compression      | SSL/TLS (Secure Sockets Layer), JPEG/PNG Compression, MPEG (video encoding), ASCII/Unicode (text encoding), XML/JSON Parsing |
| **Layer 7 – Application** | User-facing applications and services           | HTTP/HTTPS (web browsing), DNS (Domain Name System), FTP (File Transfer Protocol), SMTP/IMAP/POP3 (email protocols), IEEE 1722.1 (AVDECC for AVB/TSN) |
