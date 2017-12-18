/***************************************************
  Adafruit MQTT Library ESP8266 Example
 
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <creditentials.h>


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "kerizco"
#define WLAN_PASS       "biscarrosse"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Keriz"
#define AIO_KEY         "fd55877846d34d00a5563bf7b7cd8dad"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'time' for subscribing to current time
Adafruit_MQTT_Subscribe timefeed = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

// Setup a feed called 'slider' for subscribing to changes on the slider
Adafruit_MQTT_Subscribe message = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/message", MQTT_QOS_1);
/*************************** Sketch Code ************************************/

int sec;
int minu;
int hour;

int timeZone = 1; // utc-4 eastern daylight time (nyc)
int messageRecu = 0;
// initialize the library with the numbers of the interface pins
#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

 
int state = 0;
int ledOFF = 0;
int displayState = 0; // 0 = no new message 1 = new message
char message_actual[255];
char message_new[255];
// digital pin 5
#define LED_PIN 16
#define BUTTON_PIN 10

void timecallback(uint32_t current) {
  display.clearDisplay();
  display.setTextSize(3);

  // adjust to local time zone
  current += (timeZone * 60 * 60);

  // calculate current time
  sec = current % 60;
  current /= 60;
  minu = current % 60;
  current /= 60;
  hour = current % 24;
  display.setCursor(0,0);
  // print hour
  if(hour == 0 || hour == 12)
    display.print("12");
  else{
  if(hour < 12)
    display.print(hour);
  else
    display.print(hour - 12);}
  //display.setCursor(20,0);
  // print mins
  display.print(":");
  if(minu < 10) display.print("0");
  display.print(minu);
  
  if(hour < 12)
    display.print("am");
  else
    display.print("pm");
  display.setCursor(0,24);
  display.setTextSize(2);
  display.print(message_actual);
  display.display();
}

void messagecallback(char *data, uint16_t len) {
  strcpy(message_new, data);
  messageRecu = 1;
}


void setup() {
  Serial.begin(115200);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  delay(10);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  
  display.setCursor(0,0);
  display.display();
 
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  timefeed.setCallback(timecallback);
  message.setCallback(messagecallback);
  
  // Setup MQTT subscription for time feed.
  mqtt.subscribe(&timefeed);
  mqtt.subscribe(&message);
}


uint32_t x=0;

void loop() {
  MQTT_connect();
  
  mqtt.processPackets(1000);

  if(messageRecu == 1){ 
   state = !state;
   digitalWrite(LED_PIN, state);
    if (digitalRead(BUTTON_PIN) == LOW){
      messageRecu = 0;
     
      strcpy(message_actual, message_new);
      digitalWrite(LED_PIN, LOW);  
    }
  }

  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
