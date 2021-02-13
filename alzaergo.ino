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
NeoSWSerial pin11(12, 9); // RX, TX
//SoftwareSerial pin12(12, 7); // RX, TX
#define BUFFER_SIZE 512
int pin10_counter = 0;
int buffer[BUFFER_SIZE+10];
int buffer_counter = 0;
int pin11_counter = 0;
int pin12_counter = 0;

static void handlePin10Rx( uint8_t c )
{
    //Serial.print("p:");
    //Serial.print(c, HEX);
    if (buffer_counter >= BUFFER_SIZE) return;
    //buffer[buffer_counter] = 10;
    buffer[buffer_counter] = c;
    buffer_counter = buffer_counter + 1;
}

static void handlePin11Rx( uint8_t c )
{
    //Serial.print("b:");
    //Serial.print(c, HEX);
    //buffer[buffer_counter] = 11;
    if (buffer_counter >= BUFFER_SIZE) return;
    buffer[buffer_counter] = c;
    buffer_counter = buffer_counter + 1;
}

void setup()
{
  pinMode(13, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }


  Serial.println("Goodnight moon2!");
  buffer[0] = 0;


  pin10.attachInterrupt(handlePin10Rx);
  pin10.begin(9600);

  //pin11.attachInterrupt(handlePin11Rx);
  //pin11.begin(9600);

  // set the data rate for the SoftwareSerial port
  //pin10.begin(9600);
  //pin11.begin(9600);
  //pin12.begin(9600);
  //mySerial.println("Hello, world?");
}
int incomingByte = 0;
/*

void printFormattedSerial(SoftwareSerial *pin, int counter) {
    //Serial.print("pin10: ");
    incomingByte = pin->read();
    Serial.print(counter);
    Serial.print(":");
    if (incomingByte < 16) {
      Serial.print("0");
    }
    Serial.println(incomingByte, HEX);
}
*/



void loop() // run over and over
{
  if (buffer_counter >= BUFFER_SIZE) {
    for (int i = 0; i< buffer_counter; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    buffer_counter = 0;
  }

    //Serial.print('a');
/*
  if (pin10.available()) {
    printFormattedSerial(&pin10, 10);
  }


  if (pin11.available()) {
    printFormattedSerial(&pin11, 11);
  }
*/

}
