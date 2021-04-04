#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "arduino-mock/Serial.h"



typedef Serial_ HardwareSerial;

#include "../src/AlzaET1Ng.h"

using ::testing::Return;
#define KEY_PIN 22

class BasicTestFixture : public ::testing::Test {
    protected:
    ArduinoMock *arduinoMock;
    SerialMock *serialMock;

    void SetUp() {
        arduinoMock = arduinoMockInstance();
        serialMock = serialMockInstance();
        EXPECT_CALL(*arduinoMock, pinMode(KEY_PIN, OUTPUT));
        EXPECT_CALL(*arduinoMock, digitalWrite(KEY_PIN, 0));
        EXPECT_CALL(*serialMock, begin(9600));
    }

    void TearDown() {
        releaseSerialMock();
        releaseArduinoMock();
    }
};

TEST_F(BasicTestFixture, holdCommand) {
    EXPECT_CALL(*arduinoMock, digitalWrite(KEY_PIN, 1));
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    AlzaControl.holdCommand(AlzaET1Ng::Commands::Down);
    ASSERT_EQ(AlzaControl.getNextCommand(), AlzaET1Ng::Commands::Down);
}

TEST_F(BasicTestFixture, holdCommandStatus) {
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    EXPECT_CALL(*arduinoMock, digitalWrite(KEY_PIN, 0));
    AlzaControl.holdCommand(AlzaET1Ng::Commands::Status);
    ASSERT_EQ(AlzaControl.getNextCommand(), AlzaET1Ng::Commands::Status);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}