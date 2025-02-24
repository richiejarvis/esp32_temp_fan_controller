// esp32_temp_fan_controller - Richie Jarvis - richie@helkit.com
// Version: v0.0.1 - 2025-02-24
// Github: https://github.com/richiejarvis/
// Version History
// v0.0.1 - Initial Release

// Check the temperature, and if above the set-point (25 degrees), start the 12v fans via the relay

// A useful alternative to the Pin 12 to GND reset
#define CONFIG_VERSION "017"
#define CONFIG_VERSION_NAME "v0.0.1"

// #include <IotWebConf.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// #include <WiFi.h>
#include <Wire.h>
// #include <SPI.h>
// #include "time.h"
// #include <HTTPClient.h>
// #include <RingBuf.h>

// // IotWebConf max lengths
// #define STRING_LEN 255
// #define NUMBER_LEN 32
// IotWebConf -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to build an AP. (E.g. in case of lost password)
#define CONFIG_PIN 12
// IotWebConf -- Status indicator pin.
////      First it will light up (kept LOW), on Wifi connection it will blink,
////      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN 13

// Setup the Adafruit library
Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
// Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
// Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

// Store Fan Status 
boolean current_Fan_Status = false;

// Here are my Variables
// Constants
// const char thingName[] = "TempSensor";
// const char wifiInitialApPassword[] = "797ph0NE*$52699!";

const int triggerTemp = 25;

// Other Variables
// char iwcThingName[STRING_LEN] = "TempSensor";


// Setup everything...
void setup() {
  Serial.begin(115200);
  debugOutput("INFO: Starting TempSensor " + (String)CONFIG_VERSION_NAME);
    delay(1000);
  /* Initialise the sensor */
  // This part tries I2C address 0x77 first, and then falls back to using 0x76.
  // If there is no I2C data with these addresses on the bus, then it reports a Fatal error, and stops
  if (!bme.begin(0x77, &Wire)) {
    debugOutput("INFO: BME280 not using 0x77 I2C address - checking 0x76");
    if (!bme.begin(0x76, &Wire)) {
      debugOutput("FATAL: Could not find a valid BME280 sensor, check wiring!");
      while (1) delay(10);
    }
  }
  debugOutput("INFO: Initialisation completed");
}


//  This is where we do stuff again and again...
void loop() {
    // Check current temp
    checkTemp(triggerTemp);
    // Delay a second
    delay(1000);
  }


}

boolean checkTemp(int temp) {
  boolean status = false;
  sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  // Store the values from the BME280 in local vars
  float temperature = temp_event.temperature;
  // Store whether the sensor was connected
  // Sanity check to make sure we are not underwater, or in space!
  if (temperature < -40.00 || temperature > 60) {
    debugOutput("ERROR: Sensor Failure");
    if (pressure > 1100.00 || pressure < 300.00) {
      debugOutput("ERROR: Sensor Failure");
    }
    status = false;
  }
  if (temperature < 25.00) (
    // START THE FANS!
    debugOutput("INFO: Starting Fans");
    startFans();
  ) else {
    debugOutput("INFO: Stopping Fans");
    current_Fan_Status = stopFans();
  }
  return status;
}

boolean startFans()
{
    // Turn Relay on!
    return true;
}

boolean stopFans()
{
    // Turn Relay off!

    return false;
}

// Simple Logging Output...
void debugOutput(String textToSend)
{
  String text = "t:" + (String)upTime + ":m:" + (String)ESP.getFreeHeap() + ":" + textToSend;
  Serial.println(text);
}

