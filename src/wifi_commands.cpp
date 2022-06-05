#include "wifi_commands.h"
#include "ESP8266WiFi.h"
#include "at_parser.h"

// WIFI MODE 
#define WIFI_MODE_DISABLED 0
#define WIFI_MODE_STA 1
#define WIFI_MODE_AP 2
#define WIFI_MODE_AP_STA 3

int wifi_mode = WIFI_MODE_DISABLED;

char get_wifi_mode(char * value)
{
  sprintf(value, "+CWMODE:%d", wifi_mode);

  return AT_OK;
}

char set_wifi_mode(char * value)
{
  sscanf(value, "%d", &wifi_mode);

  return AT_OK;
}

void register_wifi_commands()
{
    at_register_command((string_t)"CWMODE", (at_callback)get_wifi_mode, (at_callback)set_wifi_mode, 0, 0);
}
