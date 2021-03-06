#include <NeoSWSerial.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "AlzaET1Ng.h"


#define KEY_PIN 30
const int key_pins[] = {47, 49, 51, 53};
#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_M 2

AlzaET1Ng::ControlPanel AlzaControl(&Serial1, KEY_PIN);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup()
{
    Serial.begin(9600);
    Serial.write("Hello world");
    for(int x=0; x<4; x++)
    {
      pinMode(key_pins[x], INPUT);
    }

    u8g2.begin();


    u8g2.clearBuffer();

    u8g2.setFontMode(1);
    u8g2.setFont(u8g2_font_cu12_tr);

    u8g2.setCursor(0,15);
    u8g2.print(F("Hello world"));
    u8g2.sendBuffer();

}

void writeln(String ln) {
    u8g2.clearBuffer();
    u8g2.setCursor(0,15);
    u8g2.print(ln);
    u8g2.sendBuffer();
}

int up, down, memory;
int last_refresh = 0;

void loop()
{
    AlzaControl.handleLoop();

    up = digitalRead(key_pins[KEY_UP]);
    down = digitalRead(key_pins[KEY_DOWN]);
    memory = digitalRead(key_pins[KEY_M]);

    if (up == HIGH) {
        AlzaControl.holdCommand(AlzaET1Ng::Commands::Up);
    } else if (down == HIGH) {
        AlzaControl.holdCommand(AlzaET1Ng::Commands::Down);
    } else if (memory == HIGH) {
        AlzaControl.holdCommand(AlzaET1Ng::Commands::M);
    } else {
        AlzaControl.holdCommand(AlzaET1Ng::Commands::Status);
    }

    if (millis() - last_refresh > 250) {
        writeln(AlzaControl.getBcdDisplayAsString());
        last_refresh = millis();
    }
}
