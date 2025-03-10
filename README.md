# CPP TSN Tool

The CPP tsn tool is a collection of network protocol and interfaces.


The aim of the tool is 6-fold:

 * Re-use already existing protocol developments
 * Simulate or test the whole protocol chain End-to-End 
 * Dissect and harness data input/output at different stage of a stack
 * Fuzz the harnessed data 
 * Test and understand protocols by monitoring internal variables.
 * Allow to co-simulate with real hardware and virtualized environments.

And the code should make sure that the application does not need recompilation, 
or impede developer for this it will use:

 * Avoid dependencies
 * Use dynanic linking
 * Cache as much as possible.
