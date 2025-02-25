Framers
=======

# One protocol, different Layers

Framers are layers of translation that would allow systems to be tested 
indpedendently of their nature, which could be:

 * Standard Linux sockets
 * XDP for performances
 * DPDK for performances
 * Verilog code over Verilator
 * PCAP file 

Each of the above mentioned systems use different "core engines".

As a results of this, the netcode should/would behave independently of the
interface being used for the test or the state machine under user


# Performances
At this level, performance requirements are very loose and are only created set
by intend of the applications.

So for instance if system would need to verify the right functionallities of a
FPGA design, the Verilator translation might hinder the performance.

