# Protocol builder

This tool will generate parse a protocol database and desired inputs, which are:

 * Protocol relationship (stacking)
 * Platform FPGA/x86
 * Where to place glues

Used with external tool it can provide information about the following:

 * Hierachy of the different protocol,
 * List of all available protocols, and services as Nodes.
 * Interface glues:
   * South bound / North bound
   * Introspection interface to get internal variables information
 * Interfaces variables:
   * Expected values/range etc...
