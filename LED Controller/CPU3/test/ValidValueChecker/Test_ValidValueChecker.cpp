#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ValidValueChecker.h"

TEST(ValidValueCheckerTest, PositiveValueTest)
{
    String validValue = "A String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker(&validStrings);
    EXPECT_TRUE(valueChecker.IsValidStringValue(validValue));
}

TEST(ValidValueCheckerTest, NegativeValueTest)
{
    String validValue = "A String";
    String invalidValue = "B String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker(&validStrings);
    EXPECT_FALSE(valueChecker.IsValidStringValue(invalidValue));
}

TEST(ValidValueCheckerTest, IsConfigured)
{
    String validValue = "A String";
    String invalidValue = "B String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker(&validStrings);
    EXPECT_TRUE(valueChecker.IsConfigured());
}

TEST(ValidValueCheckerTest, IsNotConfigured)
{
    String validValue = "A String";
    String invalidValue = "B String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker();
    EXPECT_FALSE(valueChecker.IsConfigured());
}