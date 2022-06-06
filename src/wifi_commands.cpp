#include "wifi_commands.h"
#include "ESP8266WiFi.h"
#include "at_parser.h"

// WIFI MODE
#define WIFI_MODE_DISABLED 0
#define WIFI_MODE_STA 1
#define WIFI_MODE_AP 2
#define WIFI_MODE_AP_STA 3

int wifi_mode = WIFI_MODE_DISABLED;

const char *format_wl_status(wl_status_t status)
{
  switch (status)
  {
  case WL_IDLE_STATUS:
    return "IDLE";
  case WL_NO_SSID_AVAIL:
    return "NO SSID AVAIL";
  case WL_SCAN_COMPLETED:
    return "SCAN COMPLETED";
  case WL_CONNECTED:
    return "CONNECTED";
  case WL_CONNECT_FAILED:
    return "CONNECT FAILED";
  case WL_CONNECTION_LOST:
    return "CONNECTION LOST";
  case WL_WRONG_PASSWORD:
    return "WRONG PASSWORD";
  case WL_DISCONNECTED:
    return "DISCONNECTED";
  }

  return "UNKNOWN";
}

char get_wifi_mode(char *value)
{
  sprintf(value, "+CWMODE:%d", wifi_mode);

  return AT_OK;
}

char set_wifi_mode(char *value)
{
  sscanf(value, "%d", &wifi_mode);

  return AT_OK;
}

char get_wifi_status(char *value)
{
  if (wifi_mode != WIFI_MODE_STA)
  {
    return AT_ERROR;
  }

  station_status_t status = wifi_station_get_connect_status();
  sprintf(value, "+CWSTATE:%d", status);

  return AT_OK;
}

char print_wl_status(wl_status_t status)
{
  Serial.println(format_wl_status(status));
  uint8_t startTime = millis();

  if (status != WL_CONNECTED || status != WL_CONNECT_FAILED)
  {
    while (millis() - startTime < 5000)
    {
      wl_status_t new_status = WiFi.status();

      if (new_status != status)
      {
        status = new_status;
        Serial.println(format_wl_status(status));
      }

      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED)
      {
        return AT_ERROR;
      }

      delay(100);
    }

    return AT_OK;
  }
}

char get_station_settings(char *value)
{
  sprintf(value, "+CWJAP:%s,%s,%d,%d", WiFi.SSID().c_str(), WiFi.BSSID(), WiFi.channel(), WiFi.RSSI());

  return AT_OK;
}

char set_station_settings(char *value)
{
  char ssid[50];
  char pwd[128];
  uint8_t bssid[50];

  if (wifi_mode != WIFI_MODE_STA)
  {
    wifi_mode = WIFI_MODE_STA;
  }

  sscanf(value, "\"%[^\"]\",\"%[^\"]\",%s",
         ssid,
         pwd,
         bssid);

  wl_status_t status = WiFi.begin(ssid, pwd);

  return print_wl_status(status);
}

char connect_station(char *value)
{
  wl_status_t status = WiFi.begin();

  return print_wl_status(status);
}

void register_wifi_commands()
{
  at_register_command((string_t) "CWMODE", (at_callback)get_wifi_mode, (at_callback)set_wifi_mode, 0, 0);
  at_register_command((string_t) "CWSTATE", (at_callback)get_wifi_status, 0, 0, 0);
  at_register_command((string_t) "CWJAP", (at_callback)get_station_settings, (at_callback)set_station_settings, 0, (at_callback)connect_station);
}
