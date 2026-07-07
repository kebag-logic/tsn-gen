/*
 * SPDX-FileCopyrightText: 2026 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2026 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */

#include <parser_common_test.h>
#include <tsn/validator.h>

using namespace tsn;

#include <string>
#include <vector>

namespace {

const std::string kDir =
    std::string(PARSER_TESTS_RES_PATH) + "/0050_validator/";

size_t validate(const std::string& file)
{
    ProtocolValidator v;
    std::vector<ValidationFinding> findings;
    return v.validateProtocolFile(kDir + file, findings);
}

bool hasMessageContaining(const std::string& file, const std::string& needle)
{
    ProtocolValidator v;
    std::vector<ValidationFinding> findings;
    v.validateProtocolFile(kDir + file, findings);
    for (const auto& f : findings) {
        if (f.message.find(needle) != std::string::npos) return true;
    }
    return false;
}

} /* namespace */

TEST(Validator, ConformingFilePasses)
{
    EXPECT_EQ(validate("good.yaml"), 0u);
}

TEST(Validator, ForeignTopLevelKeyRejected)
{
    EXPECT_GT(validate("bad_foreign_key.yaml"), 0u);
    EXPECT_TRUE(hasMessageContaining("bad_foreign_key.yaml", "message"));
}

TEST(Validator, MissingServiceRejected)
{
    EXPECT_GT(validate("bad_no_service.yaml"), 0u);
    EXPECT_TRUE(hasMessageContaining("bad_no_service.yaml", "service"));
}

TEST(Validator, OversizeFieldRejected)
{
    EXPECT_GT(validate("bad_size.yaml"), 0u);
    EXPECT_TRUE(hasMessageContaining("bad_size.yaml", "1..4096"));
}

TEST(Validator, BadDirectionRejected)
{
    EXPECT_GT(validate("bad_dir.yaml"), 0u);
    EXPECT_TRUE(hasMessageContaining("bad_dir.yaml", "dir"));
}

TEST(Validator, ServiceWithoutVarsOrEntitiesRejected)
{
    EXPECT_GT(validate("bad_empty_service.yaml"), 0u);
    EXPECT_TRUE(hasMessageContaining("bad_empty_service.yaml",
                                     "vars' and/or 'entities"));
}

TEST(Validator, FindingsCarryLineNumbers)
{
    ProtocolValidator v;
    std::vector<ValidationFinding> findings;
    v.validateProtocolFile(kDir + "bad_size.yaml", findings);
    ASSERT_FALSE(findings.empty());
    /* the oversize 'size:' is on line 4 of the fixture */
    bool anyLine = false;
    for (const auto& f : findings) if (f.line > 0) anyLine = true;
    EXPECT_TRUE(anyLine);
}

TEST(Validator, MissingDirectoryReported)
{
    ProtocolValidator v;
    std::vector<ValidationFinding> findings;
    EXPECT_GT(v.validateDirectory(kDir + "does_not_exist", findings), 0u);
}
