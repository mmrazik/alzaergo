/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 2 (connect to TX of other device)
 * TX is digital pin 3 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
//#include <SoftwareSerial.h>
#include <NeoSWSerial.h>

NeoSWSerial pin10(10, 8); // RX, TX
#define KEY_PIN 13


static void handlePin10Rx( uint8_t c )
{
    Serial.print(c, HEX);
    Serial.print(" ");
}

void setup()
{
  pinMode(KEY_PIN, OUTPUT);
  digitalWrite(KEY_PIN, LOW);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }


  Serial.println("Goodnight moon2!");


  pin10.attachInterrupt(handlePin10Rx);
  pin10.begin(9600);
}


int in = 0;
#define UP 0x77 //W
#define DOWN 0x73 //S
#define ENTER 0xd
#define SPACE 0x20
#define BUTTON_DURATION 600
#define DELAY_BETWEEN_WRITES 7
int direction = 0;
unsigned long action_start;
unsigned long last_write;


void loop() // run over and over
{

  if ((direction == UP) && ((millis() - last_write) >= DELAY_BETWEEN_WRITES)) {
    pin10.write("\xa5\x00\x20\x01\x21");
    last_write = millis();
  }

  if ((millis() - action_start) >= BUTTON_DURATION) {
    direction = 0;
    digitalWrite(KEY_PIN, LOW);
  }

  if (Serial.available()) {
    in = Serial.read();
    Serial.println(in, HEX);

    if (in == SPACE) {
       Serial.println("status");
      // A5 0 0 1 1
       pin10.write("\xa5\x00\x00\x01\x01");
       in = 0;
    }

    if (in == UP) {
      Serial.println("up");
      direction = UP;
      action_start = millis();
      digitalWrite(KEY_PIN, HIGH);
      // A5 0 20 1 21  Move up
      pin10.write("\xa5\x00\x20\x01\x21");
      last_write = millis();
    }

    if (in == DOWN) {
      Serial.println("down");
      pin10.write("\xa5\x00\x40\x01\x41");
      pin10.write(0xA5);
      pin10.write((uint8_t) 0x0);
      pin10.write(0x40);
      pin10.write(0x1);
      pin10.write(0x41);
      in = 0;
    }
  }
}
