/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace tsn {

/**
 * @brief Runtime context handed to an ILogicModule on encode/decode.
 *
 *  A context owns the ordered, named fields of one layer — the same
 *  fields the YAML declared, each with its bit width. Modules read and
 *  write those fields by short name through getValue()/setValue(); the
 *  stack runtime packs the wire bytes from the field list after
 *  onEncode() and repopulates it before onDecode().
 *
 *  Adjacency (upper()/lower()) lets a module reach neighbouring layers,
 *  e.g. an AECP length field that must count the octets of the layers
 *  stacked above it.
 */
class LayerContext {
public:
    /** One packed field: its short name, current value, and bit width. */
    struct Field {
        std::string name;
        uint64_t value;
        uint32_t bits;
    };

    LayerContext() = default;
    explicit LayerContext(std::string serviceName)
        : mServiceName(std::move(serviceName)) {}
    virtual ~LayerContext() = default;

    const std::string& getServiceName() const { return mServiceName; }

    LayerContext* upper() const { return mUpper; }
    LayerContext* lower() const { return mLower; }
    void setUpper(LayerContext* u) { mUpper = u; }
    void setLower(LayerContext* l) { mLower = l; }

    /* ---------------- field storage ---------------- */

    /** Append a field in wire order. Used by the runtime to load a layer. */
    void addField(const std::string& name, uint64_t value, uint32_t bits)
    {
        mFields.push_back(Field{name, value, bits});
    }

    /** Drop all fields (runtime reuse between encode and decode). */
    void clearFields() { mFields.clear(); }

    const std::vector<Field>& fields() const { return mFields; }

    /**
     * @brief Read a field value by short (unqualified) name.
     * @return true on success; false if the name is unknown at this layer.
     */
    bool getValue(const std::string& name, uint64_t& out) const
    {
        for (const Field& f : mFields) {
            if (f.name == name) {
                out = f.value;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Write a field value by short name. Only existing fields can
     *        be written — a module cannot invent a field that the YAML
     *        layout did not declare.
     * @return true on success; false if the name is unknown.
     */
    bool setValue(const std::string& name, uint64_t value)
    {
        for (Field& f : mFields) {
            if (f.name == name) {
                f.value = value;
                return true;
            }
        }
        return false;
    }

    /* ---------------- size helpers ---------------- */

    /** Total bytes this layer occupies on the wire (bits padded up). */
    size_t byteSize() const
    {
        size_t bits = 0;
        for (const Field& f : mFields) {
            bits += f.bits;
        }
        return (bits + 7) / 8;
    }

    /**
     * @brief Bytes occupied by the fields strictly after @p name in this
     *        layer. Used by length fields (e.g. AECP control_data_length,
     *        which counts the octets following it).
     * @return false when @p name is not a field of this layer.
     */
    bool bytesAfter(const std::string& name, size_t& outBytes) const
    {
        size_t bits = 0;
        bool seen = false;
        for (const Field& f : mFields) {
            if (seen) {
                bits += f.bits;
            } else if (f.name == name) {
                seen = true;
            }
        }
        if (!seen) {
            return false;
        }
        outBytes = (bits + 7) / 8;
        return true;
    }

    /** Total bytes of every layer stacked above this one. */
    size_t bytesAbove() const
    {
        size_t total = 0;
        for (const LayerContext* u = mUpper; u != nullptr; u = u->mUpper) {
            total += u->byteSize();
        }
        return total;
    }

private:
    std::string mServiceName;
    LayerContext* mUpper = nullptr;
    LayerContext* mLower = nullptr;
    std::vector<Field> mFields;
};

} /* namespace tsn */
