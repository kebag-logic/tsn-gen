Layer 1 (physical)
==================

The physical layer is not modelled as a packet protocol: it carries no
bit-packed PDU that the YAML schema describes. Placeholder stubs that used
to live here (MII/GMII/RGMII/PCS/PMA/RS) were removed because they did not
conform to the protocol schema and served no runtime purpose.

When a medium needs representing for simulation — Ethernet, wireless, USB,
etc. — add a schema-conforming `.yaml` here (see
[../../docs/schema/protocol.schema.json](../../docs/schema/protocol.schema.json))
or wire the medium through the `sim/` co-simulation instead.
