# Protocol Parser

The `protocol_parser` shared library (`libprotocol_parser.so`) is responsible
for discovering YAML protocol definition files in a directory tree, parsing
them, and exposing the results as in-memory typed databases.

---

## Public API overview

```
ProtocolParser
    │
    ├── parse()                         Walk base path, parse all .yaml files
    │
    ├── getVarDatabase()                → Database<Var>
    │       └── getElement(name)        → const Var*
    │
    └── getInterfaceDatabase()          → Database<ProtocolInterface>
            └── getElement(name)        → const ProtocolInterface*
```

---

## ProtocolParser

Header: `parser/inc/protocol_parser.h`

```cpp
class ProtocolParser {
public:
    explicit ProtocolParser(const std::string& baseProtoPath);

    const ProtocolParserErr parse();

    const Database<Var>&              getVarDatabase()       const;
    const Database<ProtocolInterface>& getInterfaceDatabase() const;

    const std::string& getVersion() const;
    const std::string& getBasePath() const;
};
```

### Construction

```cpp
ProtocolParser parser("/path/to/protocols");
```

The constructor stores the base path. No I/O is performed at this stage.

### parse()

Recursively walks `baseProtoPath` (up to 16 directory levels deep), finds
every `.yaml` file, and calls `Protocol::parseProtocolFile()` on each one.
Variables and interfaces extracted from each file are inserted into the shared
`Database<Var>` and `Database<ProtocolInterface>` with fully qualified names
(see [Naming](#naming)).

Returns `PROTOPARSER_SUCCESS` on success, or one of:

| Error code | Cause |
|-----------|-------|
| `PROTOPARSERERR_INVALID_INPUT` | Base path does not exist or is not a directory |
| `PROTOPARSERERR_UNEXPECTED` | A YAML file failed to parse |

### Error codes

```cpp
enum {
    PROTOPARSER_SUCCESS,
    PROTOPARSERERR_UNKNOWN,
    PROTOPARSERERR_INVALID_INPUT,
    PROTOPARSERERR_UNEXPECTED,
    PROTOPARSERERR_UNDEFINED,
    PROTOPARSERERR_EXISTS,
};
```

---

## Protocol

Header: `parser/inc/protocol.h`  
Source: `parser/src/protocol.cpp`

Parses a single YAML file. Called internally by `ProtocolParser`; not part of
the public library API.

Key responsibilities:

1. Open and mmap the file.
2. Parse YAML with rapidyaml into an in-memory tree.
3. Validate the root `service` key.
4. Iterate `vars` to build `Var` objects.
5. Iterate `entities → interfaces` to build `ProtocolInterface` objects.
6. Insert everything into the databases passed in by `ProtocolParser`.

---

## Var

Header: `parser/inc/var.h`

```cpp
class Var {
public:
    Var(const std::string& name, uint32_t size = 0,
        std::vector<int32_t> expectedValues = {});

    uint32_t                       getSize()           const;
    const std::vector<int32_t>&    getExpectedValues() const;
    const VarErr getVarName(std::string& name)         const;
};
```

| Field | Type | Description |
|-------|------|-------------|
| `mName` | `std::string` | Qualified name (e.g. `simple_service::simple_var`) |
| `mSize` | `uint32_t` | Width in **bits** |
| `mExpectedValues` | `std::vector<int32_t>` | Allowed values; empty means unconstrained |

---

## ProtocolInterface

Header: `parser/inc/protocol_interface.h`

```cpp
class ProtocolInterface {
public:
    enum IfDirections { IN, OUT };

    ProtocolInterface(const std::string& interfaceName,
                      IfDirections dir,
                      std::vector<std::string> varRefs = {});

    const std::string&              getName()       const;
    IfDirections                    getDirection()  const;
    const std::vector<std::string>& getVarRefs()    const;
};
```

| Field | Description |
|-------|-------------|
| `mInterfaceName` | Fully qualified name (see [Naming](#naming)) |
| `mDirection` | `IN` = entity receives data; `OUT` = entity emits data |
| `mVarRefs` | Ordered list of qualified var names referenced by this interface |

**Direction semantics.** From the traffic generator's perspective, `IN`
interfaces are those where the device-under-test (DUT) *receives* packets.
This is the primary mode for test traffic injection. `OUT` interfaces represent
data the DUT produces.

---

## Database\<T\>

Header: `parser/inc/database.h`

A generic keyed store backed by `std::map<std::string, T>`. Two instantiations
are provided:

- `Database<Var>` — the var registry
- `Database<ProtocolInterface>` — the interface registry

```cpp
template <typename T>
class Database {
public:
    const DatabaseErr addUniqueElement(const std::string& name, const T& obj);
    const T*          getElement(const std::string& name) const; // nullptr if absent
    size_t            size() const;

    template <typename Fn>
    void forEach(Fn&& fn) const;  // fn(const std::string& name, const T& obj)
};
```

`getElement` returns a raw pointer to the stored object, valid for the
lifetime of the `ProtocolParser` that owns the database.

---

## Naming

All database keys use the **fully qualified name** scheme:

```
<service>::<var_name>                        (Var)
<service>::<entity>::<interface_name>        (ProtocolInterface)
```

Examples from `simple.yaml` (service `simple_service`):

| Key | Type |
|-----|------|
| `simple_service::simple_var` | `Var` |
| `simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE` | `ProtocolInterface` |

When a `var_ref` inside an interface references a var by its short name
(e.g. `simple_var`), the parser prepends the current service name to produce
the qualified key used in the database lookup.

---

## Depth limit

`ProtocolParser::listProtocolFiles` enforces a maximum recursion depth of
**16 directory levels** below the base path. Files nested deeper are silently
ignored. The test resource `0002_max_depth_reached_out` (34 levels) verifies
this behaviour.

---

## Example usage

```cpp
#include <protocol_parser.h>

ProtocolParser parser("/path/to/protocols");

ProtocolParserErr err = parser.parse();
if (err.getErrorCode() != ProtocolParserErr::PROTOPARSER_SUCCESS) {
    // handle error
}

// Look up a specific variable
const Var* v = parser.getVarDatabase()
                     .getElement("simple_service::simple_var");
if (v) {
    uint32_t bits = v->getSize();                      // 4
    auto& allowed = v->getExpectedValues();            // {0, 1, 2}
}

// Iterate all interfaces
parser.getInterfaceDatabase().forEach(
    [](const std::string& name, const ProtocolInterface& iface) {
        // name  = "simple_service::SIMPLE_SERVICE_ENTITY::SIMPLE_INTERFACE"
        // iface.getDirection() == ProtocolInterface::IN
    }
);
```
