#include "gtest/gtest.h"
#include <unistd.h>
#include "gmock/gmock-matchers.h"
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

    virtual void SetUp() {
        arduinoMock = arduinoMockInstance();
        serialMock = serialMockInstance();
        EXPECT_CALL(*arduinoMock, pinMode(KEY_PIN, OUTPUT));
        EXPECT_CALL(*arduinoMock, digitalWrite(KEY_PIN, 0));
        EXPECT_CALL(*serialMock, begin(9600));
    }

    virtual void TearDown() {
        releaseSerialMock();
        releaseArduinoMock();
    }
};

class TestFixtureWithSerialData : public BasicTestFixture {
    protected:
    void SetSerialMessage(const int *response, int size) {
        EXPECT_CALL(*serialMock, available())
            .Times(size)
            .WillRepeatedly(Return(true));

        EXPECT_CALL(*serialMock, read())
            .WillOnce(Return(response[0]))
            .WillOnce(Return(response[1]))
            .WillOnce(Return(response[2]))
            .WillOnce(Return(response[3]))
            .WillOnce(Return(response[4]))
            .WillOnce(Return(response[5]));

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

TEST_F(BasicTestFixture, waitsForResponseAfterStartup) {
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    EXPECT_CALL(*serialMock, write(testing::An<uint8_t>())).Times(AlzaET1Ng::COMMAND_SIZE);
    EXPECT_CALL(*arduinoMock, millis()).Times(2);
    EXPECT_CALL(*serialMock, available()).WillOnce(Return(false));
    AlzaControl.handleLoop();
    ASSERT_EQ(AlzaControl.waitingForResponse(), true);
}


TEST_F(BasicTestFixture, waitForResponseTimesOut) {
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    EXPECT_CALL(*serialMock, write(testing::An<uint8_t>())).Times(AlzaET1Ng::COMMAND_SIZE);
    EXPECT_CALL(*arduinoMock, millis())
        .WillOnce(Return(0))
        .WillOnce(Return(0));
    EXPECT_CALL(*serialMock, available()).WillOnce(Return(false));
    AlzaControl.handleLoop();
    ASSERT_EQ(AlzaControl.waitingForResponse(), true);
    EXPECT_CALL(*arduinoMock, millis()).WillOnce(Return(AlzaET1Ng::WAIT_FOR_RESPONSE_TIMEOUT + 1));
    EXPECT_CALL(*serialMock, available()).WillOnce(Return(false));
    AlzaControl.handleLoop();
    ASSERT_EQ(AlzaControl.waitingForResponse(), false);
}


TEST_F(TestFixtureWithSerialData, handleIncomingResponse) {
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    // Corresponds to 76.5
    int message[] = {AlzaET1Ng::RESPONSE_HEADER, 0x7, 0xfd, 0x6d,  0x10, 0x81};
    SetSerialMessage(message, AlzaET1Ng::RESPONSE_SIZE);
    for (int i=0; i < AlzaET1Ng::RESPONSE_SIZE; i++) {
        AlzaControl.handleIncomingResponse();
    }
    int data[3];
    AlzaControl.getBcdDisplay(data);
    ASSERT_EQ(data[0], 0x7);
    ASSERT_EQ(data[1], 0xfd);
    ASSERT_EQ(data[2], 0x6d);

    ASSERT_EQ(AlzaControl.getHeight(), 765);

    char str_data[4];
    AlzaControl.getBcdDisplayAsString(str_data);
    ASSERT_EQ(str_data[0], '7');
    ASSERT_EQ(str_data[1], '6');
    ASSERT_EQ(str_data[2], '.');
    ASSERT_EQ(str_data[3], '5');
}


TEST_F(TestFixtureWithSerialData, handleIncomingResponseWithWrongChecksum) {
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    // Corresponds to 76.5
    int message[] = {AlzaET1Ng::RESPONSE_HEADER, 0x7, 0xfd, 0x6d,  0x10, 0x82};
    SetSerialMessage(message, AlzaET1Ng::RESPONSE_SIZE);
    for (int i=0; i < AlzaET1Ng::RESPONSE_SIZE; i++) {
        AlzaControl.handleIncomingResponse();
    }
    int data[3];
    AlzaControl.getBcdDisplay(data);
    ASSERT_EQ(data[0], 0x0);
    ASSERT_EQ(data[1], 0x0);
    ASSERT_EQ(data[2], 0x0);
}


TEST_F(TestFixtureWithSerialData, testSettingsString) {
    AlzaET1Ng::ControlPanel AlzaControl(Serial, KEY_PIN);
    // Corresponds to 76.5
    int message[] = {AlzaET1Ng::RESPONSE_HEADER, 0x6d, 0x40, 0x5b,  0x10, 0x18};
    SetSerialMessage(message, AlzaET1Ng::RESPONSE_SIZE);
    for (int i=0; i < AlzaET1Ng::RESPONSE_SIZE; i++) {
        AlzaControl.handleIncomingResponse();
    }

    ASSERT_EQ(AlzaControl.getHeight(), 0);

    char str_data[4];
    AlzaControl.getBcdDisplayAsString(str_data);
    ASSERT_EQ(str_data[0], 'S');
    ASSERT_EQ(str_data[1], '-');
    ASSERT_EQ(str_data[2], '2');
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}