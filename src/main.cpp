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

  loginfo("ESP8266 AT - WIFI / TCP/IP / MQTT\n");
  loginfo("This firmware is licensed under the MIT license\n");
  loginfo(" - (see https://github.com/kbeaugrand/ESP8266-AT-WIFI-MQTT for more info)\n");
  loginfo("Type AT+CMD? for command list\n");

  loginfo("AT Command Processor ready\n");
}

void loop()
{
  process_tcp_server();
  process_at_commands();
}