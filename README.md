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

**Command:**

```txt
AT+CMD
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
