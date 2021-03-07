#include "AlzaET1Ng.h"
#include "Arduino.h"
using namespace AlzaET1Ng;

#ifdef ALZAET1NG_THREADSAFE
    #include "task.h"
#endif



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
    case 0x50:
        return 'r';
    case 0x64:
        return 'r';
    case 0x78:
        return 't';
  }
  return '?';
}

ControlPanel::ControlPanel(HardwareSerial *hws, int key) {
    pinMode(key, OUTPUT);
    digitalWrite(key, LOW);
    keyPin = key;

    serial = hws;
    serial->begin(9600);
    nextCommand = Commands::Status;
}

bool ControlPanel::isValidResponse(int response[]) {
    unsigned int checksum = 0;
    for (int i=2; i < RESPONSE_SIZE; i++) {
        checksum = (checksum + response[i]) & 0xff;
    }
    return checksum == response[RESPONSE_SIZE];
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

int ControlPanel::bcdToMetricHeight() {
    // height in mm
    // TODO: this works only with centimeters. If the control board is configured to inches this
    // will be returning garbage
    int new_height = 0;
    for (int i = 0; i < 3; i++) {
        if (!(displayStatusString[i] >= '0' && displayStatusString[i] <= '9')) {
            return 0;
        }
        new_height = (displayStatusString[i] - '0') *10 + new_height;
    }
    //if the result is e.g. 123, it means we are above one meter and we multiply by 10
    if (new_height < 200) {
        new_height = new_height * 10;
    }
    return new_height;
}

void ControlPanel::updateRepresentations() {
    #ifdef ALZAET1NG_THREADSAFE
    taskENTER_CRITICAL();
    #endif

    // raw (7 segment display)
    displayStatus[0] = responseBuffer[2];
    displayStatus[1] = responseBuffer[3];
    displayStatus[2] = responseBuffer[4];

    // 7 segment display to ASCII
    displayStatusString[0] = bcdDigitToString(displayStatus[0]);
    displayStatusString[1] = bcdDigitToString(displayStatus[1]);
    displayStatusString[2] = bcdDigitToString(displayStatus[2]);

    int new_height = bcdToMetricHeight();
    if (new_height != 0) {
        height = new_height;
    }
    #ifdef ALZAET1NG_THREADSAFE
    taskEXIT_CRITICAL();
    #endif
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
                updateRepresentations();
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
    if (cmd == Commands::Status) {
        digitalWrite(keyPin, LOW);
    } else {
        digitalWrite(keyPin, HIGH);
    }
    nextCommand = cmd;
}

void ControlPanel::getBcdDisplay(int *data) {
    #ifdef ALZAET1NG_THREADSAFE
    taskENTER_CRITICAL();
    #endif
    data[0] = displayStatus[0];
    data[1] = displayStatus[1];
    data[2] = displayStatus[2];
    #ifdef ALZAET1NG_THREADSAFE
    taskEXIT_CRITICAL();
    #endif
}

void ControlPanel::getBcdDisplayAsString(char *data) {
    #ifdef ALZAET1NG_THREADSAFE
    taskENTER_CRITICAL();
    #endif
    data[0] = displayStatusString[0];
    data[1] = displayStatusString[1];
    data[2] = displayStatusString[2];
    #ifdef ALZAET1NG_THREADSAFE
    taskEXIT_CRITICAL();
    #endif
}

int ControlPanel::getHeightInMm() {
    #ifdef ALZAET1NG_THREADSAFE
    taskENTER_CRITICAL();
    #endif
    return height;
    #ifdef ALZAET1NG_THREADSAFE
    taskEXIT_CRITICAL();
    #endif
}