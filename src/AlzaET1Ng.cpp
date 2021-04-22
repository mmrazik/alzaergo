#define ALZAET1NG_THREADSAFE
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
    case 0x50:
        return 'r';
    case 0x31:
        return 'T';
    case 0x76:
        return 'H';
    case 0x5c:
        return 'o';
    case 0x78:
        return 't';
    case 0x00:
        return ' ';
  }
  return '?';
}


ControlPanel::ControlPanel(HardwareSerial &hws, int key):
  serial(hws) {
    pinMode(key, OUTPUT);
    digitalWrite(key, LOW);
    keyPin = key;
    #ifdef ALZAET1NG_THREADSAFE
        mutex = xSemaphoreCreateMutex();
        if (mutex == NULL) {
        // do something here
    }
    #endif

    //serial.begin(9600);
    serial.begin(9600, SERIAL_8N1, 16, 17);
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
    // Command is 5 bytes:
    // [command_header] 0x0 [command] 0x01 [checksum]
    // e.g. 0xa5, 0x00, 0x20, 0x01, 0x21
    serial.write((uint8_t) COMMAND_HEADER);
    serial.write((uint8_t) 0x0);
    serial.write((uint8_t) cmd);
    serial.write((uint8_t) 0x01);
    serial.write((uint8_t) cmd + 1); // this is potentially wrong but all the known commands are smaller than 254 so this will give the rigth result

    lastCommandExecution = millis();
    waitForResponse = true;
}


int ControlPanel::bcdToHeight() {
    // height is unfortunately unitless depending on the configuration of the control box
    // (holding T button for 8 seconds switches between inches and cm). This method
    // just converts whatever is displayed to an integer in the given unit

    // TODO: fix this for inches. If the display shows 72.5, we return 725 from here which is milimeters
    // when the table operates in inches and the display shows 32.4 we return 324 which is a unit that does
    // not make much sense
    int new_height = 0;

    // validate the display; it might show an error or something similar
    int i = 0;
    while (displayStatusString[i]) {
        // if we have some non-digit character it is probably error or some other status message
        if (!((displayStatusString[i] >= '0' && displayStatusString[i] <= '9') || (displayStatusString[i] == '.'))) {
            return 0;
        }
        i = i + 1;
    }

    new_height = (displayStatusString[0] - '0');
    new_height = new_height * 10 + (displayStatusString[1] - '0');
    if (displayStatusString[2] == '.') {
        new_height = new_height * 10 + (displayStatusString[3] - '0');
    } else {
        new_height = new_height * 10 + (displayStatusString[2] - '0');
        new_height = new_height * 10;
    }

    return new_height;
}


void ControlPanel::updateRepresentations() {
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreTake(mutex, portMAX_DELAY);
    #endif

    // raw (7 segment display)
    displayStatus[0] = responseBuffer[2];
    displayStatus[1] = responseBuffer[3];
    displayStatus[2] = responseBuffer[4];

    // 7 segment display to ASCII
    int position = 0;
    displayStatusString[position++] = bcdDigitToString(displayStatus[0]);
    displayStatusString[position++] = bcdDigitToString(displayStatus[1]);

    // If the middle character is '-' and the first one is '5', we are in the settings
    // The 7 segment display shows '5' and 'S' as the same character but they are not
    // the same. Be a bit smarter here and change the '5' to 'S'
    if ((displayStatusString[1] == '-') && (displayStatusString[0] == '5')) {
        displayStatusString[0] = 'S';
    }

    // if the middle digit has top most bit set to 1, it means we have decimal point
    if ((displayStatus[1] & 0b10000000) == 0b10000000) {
        displayStatusString[position++] = '.';
    }
    displayStatusString[position++] = bcdDigitToString(displayStatus[2]);
    displayStatusString[position] = 0;

    int new_height = bcdToHeight();
    if (new_height != 0) {
        height = new_height;
    }
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreGive(mutex);
    #endif
}

void ControlPanel::handleIncomingResponse() {
    if (serial.available()) {
        int c = serial.read();
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
                #ifdef ALZAET1NG_THREADSAFE
                // This is just an assumption that when we are running in the THREADSAFE mode
                // we run on a dedicated core/task. In that case we need to call delay every now and then
                // otherwise the watchdog timer will be triggered and will kill the task

                // TODO: try to figure out if there is a way to just feed the watchdog and not do a 1ms delay
                // ~0.5ms delay might be sill desirable so we do not send next command immediately after the receive.
                // The originial control panel seem to be sending a command every ~7ms (which is approcimately
                // 1 command transmittion time + something on top)
                delay(1);
                #endif
            } // else invalid response -- nothing we can do here; ignore and let the controller send the nextCommand
        }
    }
}


void ControlPanel::detectStaleHeight() {

}

void ControlPanel::handleLoop() {
    #ifdef DEBUG
    if ((millis() - lastUpdate) > 50) {
        lastUpdate = millis();
        Serial.write(String(lastUpdate).c_str());
        Serial.write("\r\n");
    }
    #endif
    sendCommand(nextCommand);
    handleIncomingResponse();
    detectStaleHeight();
    evaluateTargetHeight();


    // if we are in some weird state, still waiting for a response but nothing comes in, stop waiting for response
    if ((millis() - lastCommandExecution) >  WAIT_FOR_RESPONSE_TIMEOUT) {
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
    xSemaphoreTake(mutex, portMAX_DELAY);
    #endif
    data[0] = displayStatus[0];
    data[1] = displayStatus[1];
    data[2] = displayStatus[2];
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreGive(mutex);
    #endif
}


void ControlPanel::getBcdDisplayAsString(char *data) {
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreTake(mutex, portMAX_DELAY);
    #endif
    data[0] = displayStatusString[0];
    data[1] = displayStatusString[1];
    data[2] = displayStatusString[2];
    data[3] = displayStatusString[3];
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreGive(mutex);
    #endif
}


int ControlPanel::getHeight() {
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreTake(mutex, portMAX_DELAY);
    #endif
    return height;
    #ifdef ALZAET1NG_THREADSAFE
    xSemaphoreGive(mutex);
    #endif
}


void ControlPanel::evaluateTargetHeight() {
    if (targetHeight != 0) {
        if ((nextCommand == Commands::Up) && (height >= targetHeight)) {
            holdCommand(Commands::Status);
        } else
        if ((nextCommand == Commands::Down) && (height <= targetHeight)) {
            holdCommand(Commands::Status);
        }
    }
}


void ControlPanel::setHeight(int newHeight) {
    targetHeight = newHeight;
    if (newHeight > height) {
        holdCommand(Commands::Up);
    } else
    if (newHeight < height) {
        holdCommand(Commands::Down);
    }
}

Commands ControlPanel::getNextCommand() {
    return nextCommand;
}

bool ControlPanel::waitingForResponse() {
    return waitForResponse;
}
