#include "AlzaET1Ng.h"
#include "Arduino.h"
using namespace AlzaET1Ng;



const char AlzaET1Ng::bcdDigitToString(int bcd_code)
{
  bcd_code = bcd_code & 0x7f; // we don't care about the top-most bit
  switch (bcd_code) {
    case 0x3f:
        return '0';
    case 0x06:
        return '1';
    case 0x5b:
        return '2';
    case 0x4f:
        return '3';
    case 0x66:
        return '4';
    case 0x6d:
        return '5';
    case 0x7d:
        return '6';
    case 0x07:
        return '7';
    case 0x7f:
        return '8';
    case 0x6f:
        return '9';
    case 0x40:
      return '-';
    case 0x79:
      return 'E';
  }
  return ' ';
}

ControlPanel::ControlPanel(HardwareSerial *hws, int key) {
    pinMode(key, OUTPUT);
    digitalWrite(key, LOW);
    keyPin = key;

    serial = hws;
    serial->begin(9600);
    while (!*serial) {
        ; // wait for serial port to connect. Needed for Native USB only
    }
    nextCommand = Commands::Status;
}

bool ControlPanel::isValidResponse(int response[]) {
    unsigned int checksum = 0;
    for (int i=1; i < RESPONSE_SIZE - 1; i++) {
        checksum = (checksum + response[i]) & 0xff;
    }
    return checksum == response[RESPONSE_SIZE - 1];
}

void ControlPanel::sendCommand(Commands cmd) {
    if (waitForResponse) {
        return;
    }
    for (int i=0; i < COMMAND_SIZE; i++) {
      serial->write((uint8_t) commands[(int)cmd][i]);
    }
    lastCommandExecution = millis();
    waitForResponse = true;
}


void ControlPanel::handleIncomingResponse() {
    if (serial->available()) {
        int c = serial->read();
        if (c == RESPONSE_HEADER) {
            responseBuffer[0] = 1;
            responseBuffer[1] = c;
        } else if ((responseBuffer[0] >= 1) && (responseBuffer[0] < RESPONSE_SIZE)) {
            responseBuffer[++responseBuffer[0]] = c;
        } else {
            waitForResponse = false;
        }
        if (responseBuffer[0] == RESPONSE_SIZE) {
            waitForResponse = false;
            if (isValidResponse(responseBuffer)) {
                displayStatus[0] = responseBuffer[2];
                displayStatus[1] = responseBuffer[3];
                displayStatus[2] = responseBuffer[4];
            } // else invalid response -- nothing we can do here; ignore and let the controller send the nextCommand
        }
    }
}

void ControlPanel::handleLoop() {
    sendCommand(nextCommand);
    handleIncomingResponse();


    // if we are in some weird state, still waiting for a response but nothing comes in, stop waiting for response
    if ((millis() - lastCommandExecution) >  200) {
      waitForResponse = false;
    }
}


void ControlPanel::holdCommand(Commands cmd) {
    nextCommand = cmd;
    if (cmd == Commands::Status) {
        digitalWrite(keyPin, LOW);
    } else {
        digitalWrite(keyPin, HIGH);
    }
}

const int *const ControlPanel::getBcdDisplay() {
    return displayStatus;
}

const char *ControlPanel::getBcdDisplayAsString() {
    displayStatusString[0] = bcdDigitToString(displayStatus[0]);
    displayStatusString[1] = bcdDigitToString(displayStatus[1]);
    displayStatusString[2] = bcdDigitToString(displayStatus[2]);
    return displayStatusString;
}