# ESP8266 - WROOM-02 - AT Firmware for WIFI and MQTT

This is the AT firmware for the ESP8266 WROOM-02.
It provides a WIFI and MQTT interface with AT commands.

## AT Command Types

Generic AT command has four types:

| Type   |      Command Format      |  Description |
|----------|:-------------:|------:|
| Test Command   |  AT+=?  | Queries the Set Commands’ internal parameters and their range of values. |
| Query Command | AT+?|   Returns the current value of parameters. |
| Set Command   | AT+=<…>  |    Sets the value of user-defined parameters in commands, and runs these commands. |
| Execute Command | AT+ | Runs commands with no user-defined parameters. |

* Square brackets [ ] designate parameters that may be omitted; default value of the parameter will be used instead.

  Below are examples of entering command AT+CWJAP with some parameters omitted:

    ```txt
    AT+CWJAP="ssid","password"
    AT+CWJAP="ssid","password","11:22:33:44:55:66"
    ```

* If the parameter which is not the last one is omitted, you can give a , to indicate it.

    Example:

    ```txt
    AT+CWJAP="ssid","password",,1
    ```

* String values need to be included in double quotation marks, for example: AT+CWSAP="ESP756290","21030826",1,4.

* Escape character syntax is needed if a string contains any special characters, such as ,, " or \:

  * ``\\``: escape backslash itself
  * ``\,``: escape comma which is used to separate each parameter
  * ``\"``: escape double quotation marks which used to mark string input
  * ``\<any>``: escape ``<any>`` character means that drop backslash symbol and only use ``<any>`` character

    Example:

    ```txt
    AT+CWJAP="comma\,backslash\\ssid","1234567890"
    AT+MQTTPUB=0,"topic","\"{\"sensor\":012}\"",1,0
    ```

* The default baud rate of AT command is 115200.

* AT commands are ended with a new-line (CR-LF), so the serial tool should be set into “New Line Mode”.

## Basic AT Commands

### AT: Test AT Startup

**Command:**

```txt
AT
```

**Response:**

```txt
OK
```

### AT+RST: Restart a Module

**Command:**

```txt
AT+RST
```

**Response:**

```txt
OK
```

### AT+GMR: Check Version Information

**Command:**

```txt
AT+GMR
```

**Response:**

```txt
<AT version info>
<Bin version>

OK
```

**Parameters:**

* ``<AT version info>``: information about the AT library version
* ``<Bin version>``: firmware version

**Example:**

```txt
AT+GMR
AT version:1.0.0
Bin version:0.0.1

OK
```

### AT+CMD: List all AT commands and types supported in current firmware

**Query Command:**

```txt
AT+CMD?
```

**Response:**

```txt
+CMD:<index>,<AT command name>,<support test command>,<support query command>,<support set command>,<support execute command>

OK
```

**Parameters:**

* ``<index>``: AT command sequence number.
* ``<AT command name>``:  AT command name.
* ``<support test command>``: 0 means not supported, 1 means supported.
* ``<support query  command>``: 0 means not supported, 1 means supported.
* ``<support set   command>``: 0 means not supported, 1 means supported.
* ``<support execute  command>``: 0 means not supported, 1 means supported.

## WIFI AT Commands

### AT+CWMODE: Query/Set the Wi-Fi Mode (Station/SoftAP/Station+SoftAP)

**Command:**

```txt
AT+CWMODE?
```

**Response:**

```txt
+CWMODE:<mode>

OK
```

**Parameters:**

* ``<time>``: the duration of the device’s deep sleep. Unit: ms.ESP device will automatically wake up after the deep-sleep for as many milliseconds (ms) as ``<time>`` indicates.Upon waking up, the device calls deep sleep wake stub, and then proceeds to load application.

### AT+CWSTATE: Query the Wi-Fi state and Wi-Fi information

**Command:**

```txt
AT+CWSTATE?
```

**Response:**

```txt
+CWSTATE:<state>,<ssid>

OK
```

**Parameters:**

* ``<state>``: current Wi-Fi state.
        0: The station has not started any Wi-Fi connection.
        1: The station has connected to an AP, but does not get an IPv4 address yet.
        2: The station has connected to an AP, and got an IPv4 address.
        3: The station is in Wi-Fi connecting or reconnecting state.
        4: The station is in Wi-Fi disconnected state.
* ``<ssid>``: the SSID of the target AP.

### AT+CWJAP: Connect to an AP

**Query Command:**

Query the AP to which the Station is already connected.

```txt
AT+CWJAP?
```

**Response:**

```txt
+CWJAP:<ssid>,<bssid>,<channel>,<rssi>

OK
```

**Set Command:**

Query the AP to which the Station is already connected.

```txt
AT+CWJAP=[<ssid>],[<pwd>]
```

**Response:**

```txt
DISCONNECTED
CONNECTED

OK
```

or

```txt
DISCONNECTED

ERROR
```

**Parameters:**

* ``<ssid>``: the SSID of the target AP.
    Escape character syntax is needed if SSID or password contains special characters, such ``,``, ``"``, or ``\\``.
* ``<pwd>``: password, MAX: 63-byte ASCII.
* ``<bssid>``: the MAC address of the target AP. It cannot be omitted when multiple APs have the same SSID.
* ``<channel>``: channel.
* ``<rssi>``: signal strength.

### AT+CWRECONNCFG: Query/Set the Wi-Fi Reconnecting Configuration

**Query Command:**

```txt
AT+CWRECONNCFG?
```

**Response:**

```txt
+CWRECONNCFG:<reconncfg>

OK
```

**Set Command:**

Query the AP to which the Station is already connected.

```txt
AT+CWRECONNCFG:<reconncfg>
```

**Response:**

```txt
OK
```

**Parameters:**

* ``<reconncfg>``: the reconnecting configuration.
    0: auto-reconnect is disabled.
    1: auto-reconnect is enabled.

### AT+CWLAP: List Available APs

**Execute Command:**

```txt
AT+CWLAP
```

**Response:**

```txt
+CWLAP:<ecn>,<ssid>,<rssi>,<mac>,<channel>

OK
```

**Parameters:**

* ``<ecn>``: encryption method.
    0: OPEN
    1: WEP
    2: WPA PSK
    3: WPA2 PSK
    4: WPA WPA2 PSK
* ``<ssid>``: string parameter showing SSID of the AP.
* ``<rssi>``: signal strength.
* ``<mac>``: string parameter showing MAC address of the AP.
* ``<channel>``: channel.

### AT+CWQAP: Disconnect from an AP

**Execute Command:**

```txt
AT+CWQAP
```

**Response:**

```txt
OK
```

### AT+CWSAP: Query/Set the configuration of an ESP32 SoftAP

**Query Command:**

```txt
AT+CWSAP?
```

**Response:**

```txt
+CWSAP:<ssid>,<pwd>,<channel>

OK
```

**Set Command:**

```txt
AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>,<max conn>,<ssid hidden>
```

**Response:**

```txt
OK
```

**Parameters:**

* ``<ssid>``: string parameter showing SSID of the AP.
* ``<pwd>``: string parameter showing the password. Length: 8 ~ 63 bytes ASCII.
* ``<channel>``: channel ID.
* ``<ecn>``: encryption method; WEP is not supported.
  * 0: OPEN
  * 2: WPA_PSK
  * 3: WPA2_PSK
  * 4: WPA_WPA2_PSK
* [``<max conn>``]: maximum number of stations that SoftAP can connect. Range: [1,10].
* [``<ssid hidden>``]:
  * 0: broadcasting SSID (default).
  * 1: not broadcasting SSID.

### AT+CWLIF: Obtain IP Address of the Station That Connects to an SoftAP

**Execute Command:**

```txt
AT+CWLIF
```

**Response:**

```txt
+CWLIF:<ip addr>,<mac>

OK
```

**Parameters:**

* ``<ip addr>``: IP address of the station that connects to the SoftAP.
* ``<mac>``: MAC address of the station that connects to the SoftAP.

### AT+CWQIF: Disconnect Stations from an SoftAP

**Execute Command:**

```txt
AT+CWQIF
```

**Response:**

```txt
OK
```

### AT+CWDHCP: Enable/Disable DHCP

**Query Command:**

```txt
AT+CWDHCP?
```

**Response:**

```txt
+CWDHCP:<state>

OK
```

**Set Command:**

```txt
AT+CWDHCP=<operate>,<mode>
```

**Response:**

```txt
OK
```

**Parameters:**

* ``<operate>``:
    0: disable
    1: enable
* ``<mode>``:
    0: Station DHCP
    1: SoftAP DHCP
* ``<state>``: the status of DHCP
    Bit0:
        0: Station DHCP is disabled.
        1: Station DHCP is enabled.
    Bit1:
        0: SoftAP DHCP is disabled.
        1: SoftAP DHCP is enabled.

### AT+CIPSTA: Query/Set the IP Address of an ESP32 Station

**Query Command:**

```txt
AT+CIPSTA?
```

**Response:**

```txt
+CIPSTA:ip:<ip>
+CIPSTA:gateway:<gateway>
+CIPSTA:netmask:<netmask>

OK
```

**Parameters:**

* ``<ip>``: string parameter showing the IPv4 address of the station.
* ``<gateway>``: gateway.
* ``<netmask>``: netmask.

### AT+CWHOSTNAME: Query/Set the Host Name of an ESP32 Station

**Query Command:**

```txt
AT+CWHOSTNAME?
```

**Response:**

```txt
+CWHOSTNAME:<hostname>

OK
```

**Set Command:**

```txt
AT+CWHOSTNAME=<hostname>
```

**Response:**

```txt
OK
```

**Parameters:**

* ``<hostname>``: the host name of the Station. Maximum length: 32 bytes.

## TCP/IP AT Commands

### AT+CIPSERVER: Delete/create a TCP Server

**Query Command:**

```txt
AT+CIPSERVER?
```

**Response:**

```txt
+CIPSERVER:<mode>,<port>

OK
```

**Set Command:**

```txt
AT+CIPSERVER=<mode>,<port>
```

**Response:**

```txt
OK
```

**Parameters:**

* ``<mode>``:
    0: delete a server.
    1: create a server.
* ``<port>``: represents the port number. Range: [1024,65535].

### AT+CIPRECVLEN: Obtain Socket Data Length in Passive Receiving Mode

**Query Command:**

```txt
AT+CIPRECVLEN?
```

**Response:**

```txt
+CIPRECVLEN:<chan>,<len>

OK
```

**Parameters:**

* ``<chan>``: the channel identifier [0-3].
* ``<len>``: length of the entire data buffered for the connection.

### AT+CIPRECVDATA: Obtain Socket Data in Passive Receiving Mode

**Set Command:**

```txt
AT+CIPRECVDATA=<chan>,<r_len>
```

**Response:**

```txt
AT+CIPRECVDATA=<chan>,<a_len>,<remote_ip>,<remote_port>,<data>

OK
```

**Parameters:**

* ``<chan>``: the channel identifier [0-3].
* ``<r_len>``: length of the requested buffer.
* ``<a_len>``: length of the data you actually obtain.
    If the actual length of the received data is less than len, the actual length will be returned.
* ``<remote_ip>``: string parameter showing the remote IPv4 address.
* ``<remote_port>``: the remote port number.

### AT+CIPSTATE: Obtain the TCP Connection Information

**Query Command:**

```txt
AT+CIPSTATE?
```

**Response:**

```txt
+CIPSTATE:<channel>,<remote_ip>,<remote_port>,<local_port>

OK
```

**Parameters:**

* ``<chan>``: the channel identifier [0-3].
* ``<remote_ip>``: string parameter showing the remote IPv4 address.
* ``<remote_port>``: the remote port number.
* ``<local_port>``: the local port number.
