#include "wifi_commands.h"

#include "ESP8266WiFi.h"
#include "ESP8266WiFiType.h"

#include "at_parser.h"
#include <logging.h>

/**
 * Formats the WiFi Status.
 *
 * @param The WiFi status.
 * @return A string representing the WiFi status.
 */
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
  default:
    return "UNKNOWN";
  }
}

/**
 * Formats the WiFi encryption type.
 *
 * @param The WiFi encryption type.
 */
const uint8_t format_enc_type(uint8_t encType)
{
  switch (encType)
  {
  case ENC_TYPE_NONE:
    return 0;
  case ENC_TYPE_WEP:
    return 1;
  case ENC_TYPE_TKIP:
    return 2;
  case ENC_TYPE_CCMP:
    return 3;
  case ENC_TYPE_AUTO:
    return 4;
  default:
    return -1;
  }
}

/**
 * Gets the Wifi mode.
 *
 * @param AT+CWMODE?
 * @return +CWMODE:<mode>
 */
char get_wifi_mode(char *value)
{
  sprintf(value, "+CWMODE:%d", WiFi.getMode());

  return AT_OK;
}

/**
 * Sets the Wifi mode.
 *
 * @param AT+CWMODE=<mode>
 */
char set_wifi_mode(char *value)
{
  int mode;

  sscanf(value, "%d", &mode);

  if (WiFi.mode((WiFiMode_t)mode))
  {
    return AT_OK;
  }

  return AT_ERROR;
}

/**
 * Gets the Wifi status.
 *
 * @param AT+CWSTATE?
 * @return +CWSTATE:<state>,<ssid>
 */
char get_wifi_status(char *value)
{
  if (WiFi.getMode() != WIFI_STA)
  {
    return AT_ERROR;
  }

  switch (WiFi.status())
  {
  case WL_IDLE_STATUS:
    sprintf(value, "+CWSTATE:0,");
    return AT_OK;
  case WL_CONNECTED:
    if (!WiFi.localIP().isSet())
    {
      sprintf(value, "+CWSTATE:1,%s", WiFi.SSID().c_str());
    }
    else
    {
      sprintf(value, "+CWSTATE:2,%s", WiFi.SSID().c_str());
    }
    return AT_OK;
  case WL_CONNECTION_LOST:
    sprintf(value, "+CWSTATE:3,%s", WiFi.SSID().c_str());
    return AT_OK;
  case WL_DISCONNECTED:
    sprintf(value, "+CWSTATE:4,");
    return AT_OK;
  default:
    return AT_ERROR;
  }
}

/**
 * Prints the current WiFi status.
 * It is used by the AT+CWJAP command.
 *
 * During a maximum of 30 seconds, the module will print the current WiFi status.
 * If the module is not connected to a WiFi network, it will print the error code
 * If the module is connected to a WiFi network, it will print CONNECTED and exit.
 *
 * @param The first status.
 */
char print_wl_status(wl_status_t status)
{
  Serial.println(format_wl_status(status));
  long startTime = millis();

  while ((millis() - startTime) < 30000)
  {
    wl_status_t new_status = WiFi.status();

    if (new_status != status)
    {
      status = new_status;
      Serial.println(format_wl_status(status));
    }

    if (status == WL_CONNECTED)
    {
      return AT_OK;
    }

    delay(100);
  }

  return AT_ERROR;
}

/**
 * Query the Wifi station settings.
 *
 * @param AT+CWJAP?
 * @return +CWJAP:<ssid>,<bssid>,<channel>,<rssi>
 */
char get_station_settings(char *value)
{
  sprintf(value, "+CWJAP:%s,%s,%d,%d", WiFi.SSID().c_str(), WiFi.BSSIDstr().c_str(), WiFi.channel(), WiFi.RSSI());

  return AT_OK;
}

/**
 * Sets the Wifi station settings.
 *
 * @param AT+CWJAP=[<ssid>],[<pwd>]
 */
char set_station_settings(char *value)
{
  char ssid[50];
  char pwd[128];

  WiFi.mode(WIFI_STA);

  sscanf(value, "\"%[^\"]\",\"%[^\"]\"", ssid, pwd);

  wl_status_t status = WiFi.begin(ssid, pwd);

  return print_wl_status(status);
}

/**
 * Connects the Wifi station settings with the last SSID and password.
 *
 * @param AT+CWJAP
 */
char connect_station(char *value)
{
  wl_status_t status = WiFi.begin();

  return print_wl_status(status);
}

/**
 * Sets the Wifi auto-reconnect.
 *
 * @param AT+CWRECONNCFG=[<reconncfg>]
 */
char set_reconnect(char *value)
{
  int reconncfg;
  sscanf(value, "%d", &reconncfg);

  WiFi.setAutoReconnect(reconncfg);

  return AT_OK;
}

/**
 * Sets the Wifi auto-reconnect.
 *
 * @param AT+CWRECONNCFG?
 */
char get_reconnect(char *value)
{
  int reconncfg;
  sscanf(value, "%d", &reconncfg);

  sprintf(value, "+CWRECONNCFG:%d", WiFi.getAutoReconnect());

  return AT_OK;
}

/**
 * Prints the scan results.
 *
 * @param The network scan number.
 */
void print_scanned_networks(int numNetworks)
{
  for (int i = 0; i < numNetworks; i++)
  {
    Serial.print("+CWLAP:");
    Serial.print(format_enc_type(WiFi.encryptionType(i)));
    Serial.print(",");
    Serial.print(WiFi.SSID(i));
    Serial.print(",");
    Serial.print(WiFi.RSSI(i));
    Serial.print(",");
    Serial.print(WiFi.BSSIDstr(i).c_str());
    Serial.print(",");
    Serial.println(WiFi.channel(i));
  }
}

/**
 * Lists the Wifi access points.
 *
 * @param AT+CWLAP
 * @return +CWLAP:<ecn>,<ssid>,<rssi>,<mac>,<channel>
 */
char execute_get_list_ap(char *value)
{
  int numNetworks = WiFi.scanNetworks();

  print_scanned_networks(numNetworks);
  return AT_OK;
}

/**
 * Disconnect from an AP
 *
 * @param AT+CWQAP
 * @return OK
 */
char execute_disconnect_ap(char *value)
{
  if (WiFi.disconnect())
  {
    return AT_OK;
  }
  else
  {
    return AT_ERROR;
  }
}

/**
 * Query the Wifi Access Point.
 *
 * @param AT+CWSAP?
 * @return +CWSAP:<ssid>,<pwd>,<channel>
 */
char get_access_point_settings(char *value)
{
  sprintf(value, "+CWSAP:%s,%s,%d",
          WiFi.softAPSSID().c_str(),
          WiFi.softAPPSK().c_str(),
          WiFi.channel());

  return AT_OK;
}

/**
 * Sets the Wifi Access Point settings.
 *
 * @param AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>,<ssid hidden>
 */
char set_access_point_settings(char *value)
{
  char ssid[50];
  char pwd[128];
  int channel;
  int ecn;
  int hidden;
  int max_connection;

  bool result = false;

  sscanf(value, "\"%[^\"]\",\"%[^\"]\",%d,%d,%d,%d", ssid, pwd, &channel, &ecn, &max_connection,&hidden);

  IPAddress local_IP(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);

  LogDebug("Configuring access point...");
  LogDebug("local_IP: %s", local_IP.toString().c_str());
  WiFi.softAPConfig(local_IP, gateway, subnet);

  LogDebug("Configuring softAP...");
  LogDebug("ssid: %s", ssid);
  LogDebug("pwd: %s", pwd);
  LogDebug("channel: %d", channel);
  LogDebug("ecn: %d", ecn);
  LogDebug("max_connection: %d", max_connection);
  LogDebug("hidden: %d", hidden);

  LogDebug("Setting softAP...");
  WiFi.mode(WIFI_AP);

  if (ecn == 0)
  {
    LogDebug("Encryption: Open");
    result = WiFi.softAP(ssid, NULL, channel, hidden, max_connection);
  }
  else if (ecn == 1)
  {
    LogDebug("Encryption: WEP");
    WiFi.enableInsecureWEP(true);
    result = WiFi.softAP(ssid, pwd, channel, hidden, max_connection);
  }
  else
  {
    LogDebug("Encryption: WPA2");
    result = WiFi.softAP(ssid, pwd, channel, hidden, max_connection);
  }

  return result ? AT_OK : AT_ERROR;
}

/**
 * Obtain IP Address of the Station That Connects to an SoftAP
 *
 * @param AT+CWLIF
 * @return +CWLIF:<ip addr>,<mac>
 */
char execute_get_connected_station(char *value)
{
  int stationNum = WiFi.softAPgetStationNum();
  char mac[18] = {0};

  if (stationNum <= 0)
  {
    return AT_OK;
  }

  station_info *station = wifi_softap_get_station_info();

  do
  {
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", station->bssid[0], station->bssid[1], station->bssid[2], station->bssid[3], station->bssid[4], station->bssid[5]);

    Serial.print("+CWLIF:");
    Serial.print(IPAddress(station->ip).toString().c_str());
    Serial.print(",");
    Serial.print(String(mac).c_str());
    Serial.println();

    station = STAILQ_NEXT(station, next);
  } while (station != NULL);

  return AT_OK;
}

/**
 * Disconnect Stations from an SoftAP
 *
 * @param AT+CWQIF
 */
char execute_disconnect_station(char *value)
{
  if (WiFi.disconnect(false))
  {
    return AT_OK;
  }

  return AT_ERROR;
}

/**
 * Get DHCP setting.
 *
 * @param AT+CWDHCP?
 * @returns +CWDHCP:<state>
 */
char get_dhcp_setting(char *value)
{
  int state = 0;

  dhcp_status status = wifi_station_dhcpc_status();

  if (status == DHCP_STARTED)
    state += 1;

  status = wifi_softap_dhcps_status();

  if (status == DHCP_STARTED)
    state += 2;

  sprintf(value, "+CWDHCP:%d", state);

  return AT_OK;
}

/**
 * Enable/Disable DHCP
 *
 * @param AT+CWDHCP=<operate>,<mode>
 */
char set_dhcp_setting(char *value)
{
  int operate;
  int mode;

  sscanf(value, "%d,%d", &operate, &mode);

  switch (mode)
  {
  case 0 /* STA mode */:
    if (operate == 1)
      wifi_station_dhcpc_start();
    else
      wifi_station_dhcpc_stop();
    return AT_OK;
  case 1 /* AP mode */:
    if (operate == 1)
      wifi_softap_dhcps_start();
    else
      wifi_softap_dhcps_stop();
    return AT_OK;

  default:
    return AT_ERROR;
  }

  return AT_ERROR;
}

/**
 * Gets the Station hostname.
 *
 * @param AT+CWHOSTNAME?
 */
char set_sta_hostname(char *value)
{
  if(WiFi.setHostname(value))
  {
    return AT_OK;
  }
  else
  {
    return AT_ERROR;
  }
}

/**
 * Sets the Station hostname.
 *
 * @param AT+CWHOSTNAME=<hostname>
 */
char get_sta_hostname(char *value)
{
  sprintf(value, "+CWHOSTNAME:%s", WiFi.getHostname());
  return AT_OK;
}

/**
 * Registers the Wifi station AT commands.
 *
 */
void register_wifi_commands()
{
  at_register_command("CWMODE", (at_callback)get_wifi_mode, (at_callback)set_wifi_mode, 0, 0);
  at_register_command("CWSTATE", (at_callback)get_wifi_status, 0, 0, 0);
  at_register_command("CWJAP", (at_callback)get_station_settings, (at_callback)set_station_settings, 0, (at_callback)connect_station);
  at_register_command("CWRECONNCFG", (at_callback)get_reconnect, (at_callback)set_reconnect, 0, 0);
  at_register_command("CWLAP", 0, 0, 0, (at_callback)execute_get_list_ap);
  at_register_command("CWQAP", 0, 0, 0, (at_callback)execute_disconnect_ap);
  at_register_command("CWSAP", (at_callback)get_access_point_settings, (at_callback)set_access_point_settings, 0, 0);
  at_register_command("CWLIF", 0, 0, 0, (at_callback)execute_get_connected_station);
  at_register_command("CWQIF", 0, 0, 0, (at_callback)execute_disconnect_station);
  at_register_command("CWDHCP", (at_callback)get_dhcp_setting, (at_callback)set_dhcp_setting, 0, 0);
  at_register_command("CWHOSTNAME", (at_callback)get_sta_hostname, (at_callback)set_sta_hostname, 0, 0);
}
