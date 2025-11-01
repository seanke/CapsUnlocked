#include <gtest/gtest.h>

TEST(HelloTest, GreetsWorld) {
    EXPECT_EQ("Hello, world!", std::string("Hello, world!"));
}
