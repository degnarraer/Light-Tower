
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

#if defined(ARDUINO)
#include <Arduino.h>

#include "tests/AllTests.h"


void setup()
{
    Serial.begin(500000);
    ::testing::InitGoogleMock();
    ::testing::InitGoogleTest();
    ::testing::GTEST_FLAG(break_on_failure) = true;
}

void loop()
{
    try
    {
        if (RUN_ALL_TESTS())
        {
            ESP_LOGE("Run All Tests", "All Tests Passes!");
        }
        else
        {
            ESP_LOGE("Run All Tests", "Testing Failed!");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    // Ensure all serial output is transmitted
    Serial.flush();
    delay(1000);  // Optional delay between test runs
}

#else
int main(int argc, char **argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(break_on_failure) = true;

    int result = RUN_ALL_TESTS();

    // Always return zero-code and allow PlatformIO to parse results
    return result;
}
#endif