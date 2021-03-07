#ifndef AlzaET1Ng_h
#define AlzaET1Ng_h
#include "Arduino.h"




namespace AlzaET1Ng
{
    const char bcdDigitToString(int bcd_code);

    static const int COMMAND_SIZE = 5;
    static const int RESPONSE_SIZE = 6;
    static const int RESPONSE_HEADER  = 0x5a;
    enum Commands {
        Status = 0,
        Up = 1,
        Down = 2,
        M = 3,
        M1 = 4,
        M2 = 5,
        M3 = 6,
        T = 7,
        Reset = 8
    };

    class ControlPanel
    {
    private:
        int displayStatus[3] = {0, 0, 0};
        char displayStatusString[4] = {0, 0, 0, 0};
        int height;
        HardwareSerial *serial;
        bool isValidResponse(int response[]);
        void handleIncomingResponse();
        void updateRepresentations();
        int bcdToMetricHeight();
        Commands nextCommand;
        bool waitForResponse = false;
        unsigned long lastCommandExecution = 0;
        int responseBuffer[RESPONSE_SIZE + 1] = {0, 0, 0, 0, 0, 0, 0};
        int keyPin;
        const int commands[9][5] = {
            {0xa5, 0x00, 0x00, 0x01, 0x01},
            {0xa5, 0x00, 0x20, 0x01, 0x21},
            {0xa5, 0x00, 0x40, 0x01, 0x41},
            {0xa5, 0x00, 0x01, 0x01, 0x02},
            {0xa5, 0x00, 0x02, 0x01, 0x03},
            {0xa5, 0x00, 0x04, 0x01, 0x05},
            {0xa5, 0x00, 0x08, 0x01, 0x09},
            {0xa5, 0x00, 0x10, 0x01, 0x11},
            {0xa5, 0x00, 0x60, 0x01, 0x61}
        };

    public:
        // TODO: Add a constructor for NeoSWSerial/SoftwareSerial so this can be used with SW serial too
        ControlPanel(HardwareSerial *srl, int key);
        void sendCommand(Commands cmd);
        void handleLoop();
        void holdCommand(Commands cmd);
        const int *const getBcdDisplay();
        const char *const getBcdDisplayAsString();
        int getHeightInMm();
    };
};
#endif