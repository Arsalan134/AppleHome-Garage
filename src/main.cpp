/*
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 *
 * HAP section 8.38 Switch
 * An accessory contains a switch.
 *
 * This example shows how to:
 * 1. define a switch accessory and its characteristics (in my_accessory.c).
 * 2. get the switch-event sent from iOS Home APP.
 * 3. report the switch value to HomeKit.
 *
 * You should:
 * 1. read and use the Example01_TemperatureSensor with detailed comments
 *    to know the basic concept and usage of this library before other
 * examples。
 * 2. erase the full flash or call homekit_storage_reset() in setup()
 *    to remove the previous HomeKit pairing storage and
 *    enable the pairing with the new accessory of this new HomeKit example.
 */

#include "wifi_info.h"
#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <arduino_homekit_server.h>

#define LOG_D(fmt, ...) printf_P(PSTR(fmt "\n"), ##__VA_ARGS__);

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;

void my_homekit_setup() { arduino_homekit_setup(&config); }

void sensorSetup() { 
  delay(500); 
}

void setup() {
  Serial.begin(115200);
  sensorSetup();
  wifi_connect();
  my_homekit_setup();
  
}

void loop() {
  arduino_homekit_loop();
}