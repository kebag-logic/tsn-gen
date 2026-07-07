/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <ostream>
#include <string>

namespace tsn {

/**
 * @brief Severity levels for library diagnostics.
 *
 *  error — an operation failed and reports an error to its caller.
 *  warn  — something was skipped or ignored; results may be partial.
 *  info  — high-level progress messages.
 *  debug — per-file / per-node tracing.
 *
 *  All diagnostics go to stderr. stdout is reserved for machine-readable
 *  output (NDJSON / hex packet streams) and is never written by the
 *  libraries.
 */
enum class LogLevel {
    quiet = 0,
    error,
    warn,
    info,
    debug,
};

/** Current process-wide level. Defaults to LogLevel::warn. */
LogLevel logLevel();

void setLogLevel(LogLevel lvl);

/**
 * @brief Parse a level name ("quiet", "error", "warn", "info", "debug").
 * @return false (level unchanged) when the name is unknown.
 */
bool setLogLevel(const std::string& name);

/**
 * @brief Diagnostic stream for severity @p lvl: std::cerr when enabled
 *        by the current level, otherwise a null sink that discards writes.
 *
 *        tsn::log(tsn::LogLevel::warn) << "field skipped\n";
 */
std::ostream& log(LogLevel lvl);

} /* namespace tsn */
