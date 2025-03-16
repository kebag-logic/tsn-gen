# CPP TSN Tool

The CPP tsn tool is a collection of network protocol and interfaces.

# Goals


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

# Dependencies

Testing:
 * g++, clang, clang++, make, cmake, python3, bash, binutils


To use behave, _run-test-behave.sh_, which will create a virtualenv in your the 
folder tests/ called behav-pyvenv

Local execution can be done using the following command:

```bash
$ <package manager> install go # make sure to install go first
$ ./exec-local-test.sh
```
in the end it will provide the expected results as if running in github

