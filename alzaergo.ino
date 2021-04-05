//#include <NeoSWSerial.h>
//#include <Wire.h>
#include <U8g2lib.h>
#define ALZAET1NG_THREADSAFE
#include "src/AlzaET1Ng.h"
#include <WiFi.h>
#include "WiFiConfig.h"
#include "esp_http_server.h"





#define KEY_PIN 22
const int key_pins[] = {27, 14, 12, 13};
#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_M 2

TaskHandle_t AlzaTask;
AlzaET1Ng::ControlPanel AlzaControl(Serial2, KEY_PIN);
httpd_handle_t httpServer;

//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 32, /* reset=*/ 33);

esp_err_t get_height_handler(httpd_req_t *req)
{
    httpd_resp_send(req, String(AlzaControl.getHeight()).c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get_height = {
    .uri      = "/height",
    .method   = HTTP_GET,
    .handler  = get_height_handler,
    .user_ctx = NULL
};


httpd_handle_t startWebServer(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get_height);
    }
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}

void AlzaMainLoop( void * parameter) {
  for(;;) {
    AlzaControl.handleLoop();
  }
}

void ConnectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to "); Serial.println(ssid);

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);

    if ((++i % 16) == 0)
    {
      Serial.println(F(" still trying to connect"));
    }
  }

  Serial.print(F("Connected. My IP address is: "));
  Serial.println(WiFi.localIP());
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

    ConnectToWiFi();
    httpServer = startWebServer();

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
    u8g2.setFont(u8g2_font_helvR08_te);
    u8g2.drawStr(10, 10, WiFi.localIP().toString().c_str());
    u8g2.sendBuffer();
}