/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace tsn {

/**
 * @brief One conformance problem found in a protocol or stack YAML file.
 *
 *  The validator reports structural violations of the documented schema
 *  (docs/schema/*.json) with a file:line:col anchor so authors can jump
 *  straight to the offending node.
 */
struct ValidationFinding {
    enum Severity { ERROR, WARNING };

    std::string file;
    size_t line = 0;   /* 1-based; 0 when unknown */
    size_t col = 0;
    Severity severity = ERROR;
    std::string message;
};

/**
 * @brief Structural validator for protocol and stack YAML files.
 *
 *  Mirrors docs/schema/protocol.schema.json and stack.schema.json: it
 *  checks required keys, value types, allowed enumerations, and rejects
 *  unknown top-level keys (the foreign dialects the corpus used to carry).
 *  It does not resolve cross-file references — that is the parser's job.
 */
class ProtocolValidator {
public:
    /**
     * @brief Validate one protocol YAML file, appending findings.
     * @return number of ERROR-severity findings added.
     */
    size_t validateProtocolFile(const std::string& path,
                                std::vector<ValidationFinding>& out) const;

    /**
     * @brief Validate one stack YAML file, appending findings.
     * @return number of ERROR-severity findings added.
     */
    size_t validateStackFile(const std::string& path,
                             std::vector<ValidationFinding>& out) const;

    /**
     * @brief Recursively validate every .yaml/.yml file under @p dir as a
     *        protocol file (same tree walk the parser uses).
     * @return total number of ERROR-severity findings added.
     */
    size_t validateDirectory(const std::string& dir,
                             std::vector<ValidationFinding>& out) const;
};

} /* namespace tsn */
