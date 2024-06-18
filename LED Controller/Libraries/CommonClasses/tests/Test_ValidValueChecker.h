/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ValidValueChecker.h"

TEST(ValidValueCheckerTest, Positive_Value_Test)
{
    String validValue = "A String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker(&validStrings);
    EXPECT_TRUE(valueChecker.IsValidStringValue(validValue));
}

TEST(ValidValueCheckerTest, Negative_Value_Test)
{
    String validValue = "A String";
    String invalidValue = "B String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker(&validStrings);
    EXPECT_FALSE(valueChecker.IsValidStringValue(invalidValue));
}

TEST(ValidValueCheckerTest, Is_Configured)
{
    String validValue = "A String";
    String invalidValue = "B String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker(&validStrings);
    EXPECT_TRUE(valueChecker.IsConfigured());
}

TEST(ValidValueCheckerTest, Is_Not_Configured)
{
    String validValue = "A String";
    String invalidValue = "B String";
    ValidStringValues_t validStrings = {validValue};
    ValidValueChecker valueChecker = ValidValueChecker();
    EXPECT_FALSE(valueChecker.IsConfigured());
}