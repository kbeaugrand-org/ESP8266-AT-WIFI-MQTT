#include <Arduino.h>
#include "common.h"

#include "at_parser.h"
#include "at_command_process.h"
#include "logging.h"

#include "basic_commands.h"
#include "wifi_commands.h"
#include "tcp_ip_commands.h"

void setup()
{
  Serial.begin(115200);

  register_basic_commands();
  register_wifi_commands();
  register_tcp_ip_commands();

  Serial.println();

  LogInfo("ESP8266 AT - WIFI / TCP/IP / MQTT");
  LogInfo("This firmware is licensed under the MIT license");
  LogInfo(" - (see https://github.com/kbeaugrand/ESP8266-AT-WIFI-MQTT for more info)");
  LogInfo("Type AT+CMD? for command list");

  LogInfo("AT Command Processor ready");
}

void loop()
{
  process_tcp_server();
  process_at_commands();
}