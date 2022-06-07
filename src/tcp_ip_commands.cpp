
#include <Arduino.h>
#include "at_parser.h"

#include "common.h"
#include "tcp_ip_commands.h"

#include <ESP8266WiFi.h>

WiFiServer *tcpServer = nullptr;
bool tcpServerStarted = false;

/**
 * Sets the server port to listen for incoming TCP connections.
 *
 * @param AT+CIPSERVER=<mode>,<port>
 */
char set_server(char *value)
{
    int mode;
    int port;

    sscanf(value, "%d,%d", &mode, &port);

    if (mode == 1)
    {
        if (tcpServerStarted && tcpServer->port() != port)
        {
            Serial.println("TCP server already started on different port");
            tcpServer->close();
            delete tcpServer;
            tcpServer = nullptr;
            tcpServerStarted = false;

            tcpServer = new WiFiServer(port);
            tcpServer->begin();
            tcpServerStarted = true;

            return AT_OK;
        }
        else
        {
            Serial.println("TCP server already started but on same port.");
            return AT_OK;
        }
    }
    else if (mode == 0)
    {
        if (tcpServerStarted && tcpServer->port() == port)
        {
            Serial.println("Should stop TCP server");
            tcpServer->close();
            delete tcpServer;
            tcpServer = nullptr;
            tcpServerStarted = false;

            return AT_OK;
        }
        else
        {
            Serial.println("No TCP server to stop");
            return AT_OK;
        }
    }

    return AT_ERROR;
}

/**
 * Gets the servers.
 *
 * @param AT+CIPSERVER?
 *
 * @returns +CIPSERVER:<mode>,<port>
 */
char get_server(char *value)
{
    if (tcpServerStarted == false)
    {
        return AT_OK;
    }

    Serial.print("+CIPSERVER:");
    Serial.print(tcpServer->available() ? "1" : "0");
    Serial.print(",");
    Serial.print(tcpServer->port());

    return AT_OK;
}

void process_tcp_server()
{
}

/**
 * Registers the TCP/IP commands.
 *
 */
void register_tcp_ip_commands()
{
    at_register_command((string_t) "CIPSERVER", (at_callback)get_server, (at_callback)set_server, 0, 0);
}
