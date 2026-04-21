# Protocol builder

This tool will generate parse a protocol database and desired inputs, which are:

 * Service, their parameters, and stacking location

Used with external tool it can provide information about the following:

 * Hierachy of the different protocol,
 * List of all available protocols, and services as Nodes.
 * Interface glues:
   * South bound / North bound
   * Introspection interface to get internal variables information
 * Interfaces variables:
   * Expected values/range etc...

## Stacks and logic modules

Protocols can optionally bind a C++ logic module via a top-level
`logic: <name>` key in the service YAML. Stacks are declared as
separate YAML files listing `service` / `entity` per layer, with an
optional `bypass-logic: true` to force `PassthroughLogic` on a layer
(useful for fuzzing). See `logic/README.md` for how to write and
register a module, and `StackBuilder` / `Stack` in
`parser/inc/stack_builder.h` for the runtime wiring.
