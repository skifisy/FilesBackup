#include <gtest/gtest.h>
#include "error_code.h"

TEST(ErrorCode, use) {
    std::error_code code = backup::backup_make_error_code(0);
    EXPECT_EQ(code.message(), "Success");
    code = backup::backup_make_error_code(backup::EEOF);
    EXPECT_EQ(code.message(), "Read eof");
}