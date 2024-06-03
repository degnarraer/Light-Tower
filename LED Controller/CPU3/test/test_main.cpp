
#if defined(ARDUINO)
#include <Arduino.h>

#include "Test_CommonClasses/AllTests.h"


void setup()
{
    Serial.begin(500000);
    ::testing::InitGoogleMock();
    ::testing::InitGoogleTest();
}

void loop()
{
  if (RUN_ALL_TESTS())
  ;

  delay(1000);
}

#else
int main(int argc, char **argv)
{
    ::testing::InitGoogleMock();
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS())
    ;

    // Always return zero-code and allow PlatformIO to parse results
    return 0;
}
#endif