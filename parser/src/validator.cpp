/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <tsn/validator.h>

#include <tsn/log.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#include <ryml.hpp>

namespace tsn {

namespace {

std::string csubstrToString(ryml::csubstr s)
{
    return std::string(s.str, s.len);
}

/* Per-file validation context: owns the parser (needed for node
 * locations) and collects findings. */
struct Ctx {
    std::string path;
    ryml::Parser* parser;
    std::vector<ValidationFinding>* out;
    size_t errors = 0;

    void add(ryml::ConstNodeRef node, ValidationFinding::Severity sev,
             const std::string& msg)
    {
        ValidationFinding f;
        f.file = path;
        f.severity = sev;
        f.message = msg;
        if (!node.invalid()) {
            const ryml::Location loc = parser->location(node);
            f.line = loc.line;
            f.col = loc.col;
        }
        if (sev == ValidationFinding::ERROR) {
            ++errors;
        }
        out->push_back(std::move(f));
    }
};

bool isNonEmptyScalar(ryml::ConstNodeRef n)
{
    return !n.invalid() && n.has_val() && !n.val().empty();
}

/* Parse a YAML scalar as an unsigned integer (base 0: decimal or 0x hex).
 * Rejects a leading sign or any trailing characters. */
bool isUintScalar(ryml::ConstNodeRef n)
{
    if (n.invalid() || !n.has_val()) {
        return false;
    }
    const std::string s = csubstrToString(n.val());
    if (s.empty() || s[0] == '-' || s[0] == '+') {
        return false;
    }
    try {
        size_t consumed = 0;
        (void)std::stoull(s, &consumed, 0);
        return consumed == s.size();
    } catch (const std::exception&) {
        return false;
    }
}

long long asInt(ryml::ConstNodeRef n)
{
    return std::stoll(csubstrToString(n.val()), nullptr, 0);
}

/* Report every child key of @p map not present in @p allowed. */
void checkAllowedKeys(Ctx& ctx, ryml::ConstNodeRef map,
                      const std::set<std::string>& allowed,
                      const std::string& where)
{
    for (ryml::ConstNodeRef child : map.children()) {
        if (!child.has_key()) {
            continue;
        }
        const std::string key = csubstrToString(child.key());
        if (allowed.count(key) == 0) {
            ctx.add(child, ValidationFinding::ERROR,
                    "unknown key '" + key + "' in " + where +
                    " (not part of the protocol schema)");
        }
    }
}

void validateExpected(Ctx& ctx, ryml::ConstNodeRef expected,
                      const std::string& varName)
{
    if (!expected.is_map()) {
        ctx.add(expected, ValidationFinding::ERROR,
                "'expected' for var '" + varName + "' must be a map");
        return;
    }
    checkAllowedKeys(ctx, expected, {"value", "values", "range", "mask"},
                     "expected of var '" + varName + "'");

    if (expected.has_child("value") && !isUintScalar(expected["value"])) {
        ctx.add(expected["value"], ValidationFinding::ERROR,
                "expected.value of '" + varName +
                "' must be an unsigned integer");
    }
    if (expected.has_child("values")) {
        ryml::ConstNodeRef v = expected["values"];
        if (!v.is_seq() || v.num_children() == 0) {
            ctx.add(v, ValidationFinding::ERROR,
                    "expected.values of '" + varName +
                    "' must be a non-empty list");
        } else {
            for (ryml::ConstNodeRef item : v) {
                if (!isUintScalar(item)) {
                    ctx.add(item, ValidationFinding::ERROR,
                            "expected.values of '" + varName +
                            "' must contain unsigned integers");
                    break;
                }
            }
        }
    }
    if (expected.has_child("range")) {
        ryml::ConstNodeRef r = expected["range"];
        if (!r.is_seq() || r.num_children() != 2 ||
            !isUintScalar(r[0]) || !isUintScalar(r[1])) {
            ctx.add(r, ValidationFinding::ERROR,
                    "expected.range of '" + varName +
                    "' must be [min, max] unsigned integers");
        }
    }
    if (expected.has_child("mask")) {
        ryml::ConstNodeRef m = expected["mask"];
        if (!m.is_seq() || m.num_children() != 1 || !isUintScalar(m[0])) {
            ctx.add(m, ValidationFinding::ERROR,
                    "expected.mask of '" + varName +
                    "' must be a single-element [M] list");
        }
    }
}

void validateVar(Ctx& ctx, ryml::ConstNodeRef var)
{
    if (!var.is_map()) {
        ctx.add(var, ValidationFinding::ERROR, "each 'vars' entry must be a map");
        return;
    }
    checkAllowedKeys(ctx, var, {"var", "size", "expected"}, "a vars entry");

    std::string name = "?";
    if (!var.has_child("var") || !isNonEmptyScalar(var["var"])) {
        ctx.add(var, ValidationFinding::ERROR,
                "vars entry missing a non-empty 'var' name");
    } else {
        name = csubstrToString(var["var"].val());
    }

    if (!var.has_child("size")) {
        ctx.add(var, ValidationFinding::ERROR,
                "var '" + name + "' missing 'size'");
    } else if (!isUintScalar(var["size"])) {
        ctx.add(var["size"], ValidationFinding::ERROR,
                "var '" + name + "' size must be an unsigned integer");
    } else {
        const long long bits = asInt(var["size"]);
        /* 4096 bits (512 bytes) covers AVDECC strings/descriptor blobs while
         * still catching absurd typos. Fields wider than 64 bits carry the
         * generated value in their low 64 bits (see PacketBuilder). */
        if (bits < 1 || bits > 4096) {
            ctx.add(var["size"], ValidationFinding::ERROR,
                    "var '" + name + "' size must be in 1..4096 bits");
        }
    }

    if (var.has_child("expected")) {
        validateExpected(ctx, var["expected"], name);
    }
}

void validateInterface(Ctx& ctx, ryml::ConstNodeRef iface)
{
    if (!iface.is_map()) {
        ctx.add(iface, ValidationFinding::ERROR,
                "each interface must be a map");
        return;
    }
    checkAllowedKeys(ctx, iface, {"interface", "dir", "vars"}, "an interface");

    std::string name = "?";
    if (!iface.has_child("interface") ||
        !isNonEmptyScalar(iface["interface"])) {
        ctx.add(iface, ValidationFinding::ERROR,
                "interface missing a non-empty 'interface' name");
    } else {
        name = csubstrToString(iface["interface"].val());
    }

    if (!iface.has_child("dir")) {
        ctx.add(iface, ValidationFinding::ERROR,
                "interface '" + name + "' missing 'dir'");
    } else {
        const std::string dir = csubstrToString(iface["dir"].val());
        if (dir != "in" && dir != "out") {
            ctx.add(iface["dir"], ValidationFinding::ERROR,
                    "interface '" + name + "' dir must be 'in' or 'out' (got '" +
                    dir + "')");
        }
    }

    if (iface.has_child("vars")) {
        ryml::ConstNodeRef vars = iface["vars"];
        if (!vars.is_seq()) {
            ctx.add(vars, ValidationFinding::ERROR,
                    "interface '" + name + "' vars must be a list of var_ref");
        } else {
            for (ryml::ConstNodeRef ref : vars) {
                if (!ref.is_map() || !ref.has_child("var_ref")) {
                    ctx.add(ref, ValidationFinding::ERROR,
                            "interface '" + name +
                            "' vars entries must be '{ var_ref: name }'");
                    break;
                }
                checkAllowedKeys(ctx, ref, {"var_ref"},
                                 "a var_ref of interface '" + name + "'");
            }
        }
    }
}

void validateEntity(Ctx& ctx, ryml::ConstNodeRef ent)
{
    if (!ent.is_map()) {
        ctx.add(ent, ValidationFinding::ERROR,
                "each 'entities' entry must be a map");
        return;
    }
    checkAllowedKeys(ctx, ent, {"entity", "interfaces"}, "an entities entry");

    std::string name = "?";
    if (!ent.has_child("entity") || !isNonEmptyScalar(ent["entity"])) {
        ctx.add(ent, ValidationFinding::ERROR,
                "entities entry missing a non-empty 'entity' name");
    } else {
        name = csubstrToString(ent["entity"].val());
    }

    if (!ent.has_child("interfaces")) {
        ctx.add(ent, ValidationFinding::ERROR,
                "entity '" + name + "' missing 'interfaces'");
        return;
    }
    ryml::ConstNodeRef ifaces = ent["interfaces"];
    if (!ifaces.is_seq() || ifaces.num_children() == 0) {
        ctx.add(ifaces, ValidationFinding::ERROR,
                "entity '" + name + "' interfaces must be a non-empty list");
        return;
    }
    for (ryml::ConstNodeRef iface : ifaces) {
        validateInterface(ctx, iface);
    }
}

/* Load @p path, run @p body(root) under a fresh parser. Returns false and
 * records an ERROR when the file cannot be read or is not a YAML map. */
template <typename Body>
bool withRoot(Ctx& ctx, const std::string& path, Body&& body)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        ctx.add(ryml::ConstNodeRef{}, ValidationFinding::ERROR,
                "could not open file");
        return false;
    }
    std::stringstream ss;
    ss << in.rdbuf();
    std::string buf = ss.str();

    ryml::ParserOptions opts;
    opts.locations(true);
    ryml::EventHandlerTree evt;
    auto parser = std::make_unique<ryml::Parser>(&evt, opts);
    ryml::Tree tree = ryml::parse_in_arena(
        parser.get(), ryml::csubstr(path.c_str()),
        ryml::csubstr(buf.data(), buf.size()));

    ctx.parser = parser.get();

    if (tree.empty() || !tree.rootref().is_map()) {
        ctx.add(ryml::ConstNodeRef{}, ValidationFinding::ERROR,
                "file is empty or not a YAML map");
        return false;
    }
    body(tree.rootref());
    return true;
}

} /* anonymous namespace */

size_t ProtocolValidator::validateProtocolFile(
    const std::string& path, std::vector<ValidationFinding>& out) const
{
    Ctx ctx{path, nullptr, &out, 0};
    withRoot(ctx, path, [&](ryml::ConstNodeRef root) {
        checkAllowedKeys(ctx, root, {"service", "logic", "vars", "entities"},
                         "the top level");

        if (!root.has_child("service") || !isNonEmptyScalar(root["service"])) {
            ctx.add(root, ValidationFinding::ERROR,
                    "missing required non-empty 'service'");
        }
        if (root.has_child("logic") && !isNonEmptyScalar(root["logic"])) {
            ctx.add(root["logic"], ValidationFinding::ERROR,
                    "'logic' must be a non-empty string");
        }

        const bool hasVars = root.has_child("vars");
        const bool hasEntities = root.has_child("entities");
        if (!hasVars && !hasEntities) {
            ctx.add(root, ValidationFinding::ERROR,
                    "a protocol must define 'vars' and/or 'entities'");
        }

        if (hasVars) {
            ryml::ConstNodeRef vars = root["vars"];
            if (!vars.is_seq()) {
                ctx.add(vars, ValidationFinding::ERROR,
                        "'vars' must be a list");
            } else {
                for (ryml::ConstNodeRef var : vars) {
                    validateVar(ctx, var);
                }
            }
        }
        if (hasEntities) {
            ryml::ConstNodeRef ents = root["entities"];
            if (!ents.is_seq()) {
                ctx.add(ents, ValidationFinding::ERROR,
                        "'entities' must be a list");
            } else {
                for (ryml::ConstNodeRef ent : ents) {
                    validateEntity(ctx, ent);
                }
            }
        }
    });
    return ctx.errors;
}

size_t ProtocolValidator::validateStackFile(
    const std::string& path, std::vector<ValidationFinding>& out) const
{
    Ctx ctx{path, nullptr, &out, 0};
    withRoot(ctx, path, [&](ryml::ConstNodeRef root) {
        checkAllowedKeys(ctx, root, {"stack", "layers"}, "the top level");

        if (!root.has_child("stack") || !isNonEmptyScalar(root["stack"])) {
            ctx.add(root, ValidationFinding::ERROR,
                    "missing required non-empty 'stack'");
        }
        if (!root.has_child("layers")) {
            ctx.add(root, ValidationFinding::ERROR, "missing 'layers'");
            return;
        }
        ryml::ConstNodeRef layers = root["layers"];
        if (!layers.is_seq() || layers.num_children() == 0) {
            ctx.add(layers, ValidationFinding::ERROR,
                    "'layers' must be a non-empty list");
            return;
        }
        for (ryml::ConstNodeRef layer : layers) {
            if (!layer.is_map()) {
                ctx.add(layer, ValidationFinding::ERROR,
                        "each layer must be a map");
                continue;
            }
            checkAllowedKeys(ctx, layer,
                             {"service", "entity", "interface", "bypass-logic"},
                             "a layer");
            if (!layer.has_child("service") ||
                !isNonEmptyScalar(layer["service"])) {
                ctx.add(layer, ValidationFinding::ERROR,
                        "layer missing a non-empty 'service'");
            }
        }
    });
    return ctx.errors;
}

size_t ProtocolValidator::validateDirectory(
    const std::string& dir, std::vector<ValidationFinding>& out) const
{
    namespace fs = std::filesystem;
    size_t errors = 0;
    if (!fs::exists(dir)) {
        ValidationFinding f;
        f.file = dir;
        f.severity = ValidationFinding::ERROR;
        f.message = "directory does not exist";
        out.push_back(std::move(f));
        return 1;
    }
    for (const fs::directory_entry& e : fs::recursive_directory_iterator(dir)) {
        if (!e.is_regular_file()) {
            continue;
        }
        const std::string ext = e.path().extension().string();
        if (ext == ".yaml" || ext == ".yml") {
            errors += validateProtocolFile(e.path().string(), out);
        }
    }
    return errors;
}

} /* namespace tsn */
