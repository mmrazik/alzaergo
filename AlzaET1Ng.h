#ifndef AlzaET1Ng_h
#define AlzaET1Ng_h
#include "Arduino.h"

#ifdef ALZAET1NG_THREADSAFE
#endif



namespace AlzaET1Ng
{
    const char bcdDigitToString(int bcd_code);

    static const int COMMAND_SIZE = 5;
    static const int RESPONSE_SIZE = 6;
    static const int RESPONSE_HEADER  = 0x5a;
    static const int COMMAND_HEADER = 0xa5;
    enum Commands {
        Status = 0x0,
        Up = 0x20,
        Down = 0x40,
        M = 0x1,
        M1 = 0x02,
        M2 = 0x04,
        M3 = 0x08,
        T = 0x10,
        Reset = 0x60
    };

    class ControlPanel
    {
    private:
        int displayStatus[3] = {0, 0, 0};
        char displayStatusString[4] = {0, 0, 0, 0};
        int height = 0;
        int targetHeight = 0;

        HardwareSerial *serial;
        int keyPin;

        Commands nextCommand;
        bool waitForResponse = false;
        unsigned long lastCommandExecution = 0;
        int responseBuffer[RESPONSE_SIZE + 1] = {0, 0, 0, 0, 0, 0, 0};

        bool isValidResponse(int response[]);
        void handleIncomingResponse();
        void updateRepresentations();
        int bcdToMetricHeight();
        void evaluateTargetHeight();

    public:
        // TODO: Add a constructor for NeoSWSerial/SoftwareSerial so this can be used with SW serial too
        ControlPanel(HardwareSerial *srl, int key);
        void sendCommand(Commands cmd);
        void setHeight(int newHeight);
        void handleLoop();
        void holdCommand(Commands cmd);
        void getBcdDisplay(int *data);
        void getBcdDisplayAsString(char *data);
        int getHeightInMm();
    };
};
#endif