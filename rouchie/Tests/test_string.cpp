#include <gtest/gtest.h>
#include <string>

TEST(StringUtilsTest, StringLength) {
    std::string str = "hello";
    EXPECT_EQ(str.length(), 5);
}

TEST(StringUtilsTest, StringEmpty) {
    std::string str = "";
    EXPECT_TRUE(str.empty());
}
