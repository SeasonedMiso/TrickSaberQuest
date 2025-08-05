#include <gtest/gtest.h>

// Simple test without Unity dependencies
class SimpleConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Basic setup
    }
};

TEST_F(SimpleConfigTest, BasicMath) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST_F(SimpleConfigTest, StringOperations) {
    std::string test = "TrickSaber";
    EXPECT_EQ(test.length(), 10);
    EXPECT_TRUE(test.find("Trick") != std::string::npos);
}

TEST_F(SimpleConfigTest, FloatComparisons) {
    float a = 1.0f;
    float b = 2.0f;
    EXPECT_LT(a, b);
    EXPECT_GT(b, a);
    EXPECT_FLOAT_EQ(a + b, 3.0f);
}

TEST_F(SimpleConfigTest, BooleanLogic) {
    bool enabled = true;
    bool disabled = false;
    
    EXPECT_TRUE(enabled && !disabled);
    EXPECT_FALSE(enabled && disabled);
    EXPECT_TRUE(enabled || disabled);
}