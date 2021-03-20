//#include <NeoSWSerial.h>
//#include <Wire.h>
#include <U8g2lib.h>
#define ALZAET1NG_THREADSAFE
#include "src/AlzaET1Ng.h"


#define KEY_PIN 22
const int key_pins[] = {27, 14, 12, 13};
#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_M 2

TaskHandle_t AlzaTask;
AlzaET1Ng::ControlPanel AlzaControl(&Serial2, KEY_PIN);

//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 32, /* reset=*/ 33);


void AlzaMainLoop( void * parameter) {
  for(;;) {
    AlzaControl.handleLoop();
  }
}

void setup()
{
    u8g2.begin();


    Serial.begin(115200);
    Serial.write("Hello world");
    for(int x=0; x<4; x++)
    {
      pinMode(key_pins[x], INPUT);
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(15,10,"Hello World!");
    u8g2.sendBuffer();

    xTaskCreatePinnedToCore(
      AlzaMainLoop,
      "AlzaTask",
      10000,  /* Stack size in words */
      NULL,   /* Task input parameter */
      0,      /* Priority of the task */
      &AlzaTask,
      0); /* Core where the task should run */

}

void writeln(String ln) {
    u8g2.clearBuffer();
    u8g2.setCursor(0,15);
    u8g2.print(ln);
    u8g2.sendBuffer();
}

int up, down, memory;
unsigned long last_refresh = 0;
char displayData[] = {0, 0, 0, 0, 0};



void loop()
{

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

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    AlzaControl.getBcdDisplayAsString(displayData);
    u8g2.drawStr(35, 40, displayData);
    u8g2.sendBuffer();

}