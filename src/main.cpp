#ifndef DEBUG_WIFI
#define DEBUG_WIFI(...) Serial.println(__VA_ARGS__)
#endif

#include <Arduino.h>
#include "common.h"

#include "at_parser.h"
#include "at_command_process.h"
#include "basic_commands.h"
#include "wifi_commands.h"

void setup()
{
  Serial.begin(115200);

  register_basic_commands();
  register_wifi_commands();

  Serial.println("Ready");
}

void loop()
{
  process_at_commands();
}