
#include <Arduino.h>
#include "at_parser.h"
#include "logging.h"
#include "Array.h"

#include "common.h"
#include "tcp_ip_commands.h"
#include "at_command_process.h"

#include <ESP8266WiFi.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENT_COUNT 4
#define MAX_SERVER_COUNT 4

bool tcpServerStarted = false;
WiFiServer *tcpServer = nullptr;

Array<WiFiClient, MAX_CLIENT_COUNT> tcpClients;

char TCP_TX_BUFFER[4096] = {};
char TCP_RX_BUFFER[MAX_CLIENT_COUNT][MAX_BUFFER_SIZE] = {};
int TCP_RX_BYTES[MAX_CLIENT_COUNT] = {};

/**
 * @brief Registers the WiFi Client channel for further processing.
 *
 * @param client
 * @return int representing the channel ID for the client.
 */
int register_client(WiFiClient client)
{
    LogDebug("Registering client %s:%d in channel %d", client.remoteIP().toString().c_str(), client.remotePort(), tcpClients.size());
    tcpClients.push_back(client);

    return tcpClients.size() - 1;
}

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
        if (tcpServerStarted == true)
        {
            if (tcpServer->port() == port)
            {
                return AT_OK;
            }

            tcpServer->close();
            delete tcpServer;
            tcpServer = nullptr;
            tcpServerStarted = false;
        }

        tcpServer = new WiFiServer(port);
        tcpServer->begin();
        tcpServerStarted = true;

        return AT_OK;
    }
    else if (mode == 0)
    {
        if (tcpServerStarted == true)
        {
            if (tcpServer->port() != port)
            {
                return AT_OK;
            }

            tcpServer->close();
            delete tcpServer;
            tcpServer = nullptr;
            tcpServerStarted = false;

            return AT_OK;
        }

        return AT_OK;
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
    Serial.print(tcpServer->status());
    Serial.print(",");
    Serial.print(tcpServer->port());

    return AT_OK;
}

/**
 * Get Station IP information.
 *
 * @param AT+CIPSTA?
 * @returns
 * +CIPSTA:ip:<"ip">
 * +CIPSTA:gateway:<"gateway">
 * +CIPSTA:netmask:<"netmask">
 */
char get_sta_ip_info(char *value)
{
    sprintf(value, "+CIPSTA:%s,%s,%s",
            WiFi.localIP().toString().c_str(),
            WiFi.gatewayIP().toString().c_str(),
            WiFi.subnetMask().toString().c_str());

    return AT_OK;
}

/**
 * @brief Obtain Socket Data Length in Passive Receiving Mode
 *
 * @param AT+CIPRECVLEN?
 * @returns +CIPRECVLEN:<length>
 */
char get_server_data_len(char *value)
{
    LogTrace("%d clients are connected.", tcpClients.size());

    for (size_t i = 0; i < tcpClients.size(); i++)
    {
        Serial.printf("+CIPRECVLEN:%d,%d\n", i, TCP_RX_BYTES[i]);
    }

    return AT_OK;
}

/**
 * @brief Obtain Socket Data in Passive Receiving Mode
 *
 * @param AT+CIPRECVDATA=<chan>,<len>
 * @returns +CIPRECVDATA=<chan>,<a_len>,<remote_ip>,<remote_port>,<data>
 *
 */
char get_server_data(char *value)
{
    int len;
    int chan;

    sscanf(value, "%d,%d", &chan, &len);

    if (chan < 0 || chan > 9)
    {
        return AT_ERROR;
    }

    LogTrace("Reading %d bytes from channel %d", len, chan);

    if (len > TCP_RX_BYTES[chan])
    {
        LogTrace("Actual length of the received data of channel %d is less than %d, the actual length %d will be returned.", chan, len, TCP_RX_BYTES[chan]);
        len = TCP_RX_BYTES[chan];
    }

    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, TCP_RX_BUFFER, len);

    WiFiClient client = tcpClients[chan];

    Serial.printf("+CIPRECVDATA:%d,%d,%s,%d\n", chan, TCP_RX_BYTES[chan], client.remoteIP().toString().c_str(), client.remotePort());

    for (int i = 0; i < len; i++)
    {
        Serial.print(TCP_RX_BUFFER[chan][i]);
    }

    TCP_RX_BYTES[chan] = 0;

    free(buffer);

    return AT_OK;
}

/**
 * @brief Remove from pool connections that are closed.
 *
 */
void remove_closed_tcp_clients()
{
    Array<int, MAX_CLIENT_COUNT> chan;

    for (size_t channelID = 0; channelID < tcpClients.size(); channelID++)
    {
        if (tcpClients[channelID].status() == CLOSED || !tcpClients[channelID].connected())
        {
            LogTrace("Client on channel %d is not connected.", channelID);
            chan.push_back(channelID);
        }
    }

    for (int channelID : chan)
    {
        tcpClients.remove(channelID);
        TCP_RX_BYTES[channelID] = 0;
    }
}

/**
 * @brief Process the TCP Clients
 *  it checks for each connected clients if some data are present and add it to the buffer.
 *
 */
void process_existing_channels()
{
    remove_closed_tcp_clients();

    for (size_t channelID = 0; channelID < tcpClients.size(); channelID++)
    {
        int available = tcpClients[channelID].available();

        if (!available)
        {
            continue;
        }

        LogInfo("Reading data on channel %d.", channelID);

        int free = MAX_BUFFER_SIZE - TCP_RX_BYTES[channelID];

        if (free < available)
        {
            LogWarn("No sufficient space on buffer for channel %d. Need %d but ony %d free.", channelID, available, free);
            return;
        }

        int offset = 0;

        while (tcpClients[channelID].available())
        {
            char c = tcpClients[channelID].read();

            TCP_RX_BUFFER[channelID][TCP_RX_BYTES[channelID] + offset++] = c;

            if (c == '\n')
            {
                TCP_RX_BYTES[channelID] += offset;

                LogTrace("Got %d bytes on channel %d - Now %d bytes are waiting.", offset, channelID, TCP_RX_BYTES[channelID]);
                LogTrace("%d;%s", channelID, TCP_RX_BUFFER[channelID]);

                Serial.print("+CIPRECVLEN:");
                Serial.print(channelID);
                Serial.print(",");
                Serial.println(TCP_RX_BYTES[channelID]);
                break;
            }
        }
    }
}

/**
 * @brief Realizes the tcp connection processing.
 *  If a client is connected, it registers the client as a channel and read its incoming data.
 */
void process_tcp_server()
{
    if (!tcpServerStarted)
    {
        return;
    }

    process_existing_channels();

    if (!tcpServer->hasClient())
    {
        return;
    }

    WiFiClient client = tcpServer->available();

    if (!client)
    {
        LogTrace("TcpServer has client but unable to get the available client.");
        return;
    }

    if (!client.connected())
    {
        LogTrace("Client is not connected.");
        return;
    }

    int channelID = register_client(client);

    if (channelID < 0)
    {
        LogErr("Failed to register the client. Stopping.");
        client.stop();
    }
}

/**
 * @brief Get the connections status
 *
 * @param AT+CIPSTATE?
 * @return +CIPSTATE:<link ID>,<remote IP>,<remote port>,<local port>
 */
char get_connections_status(char *value)
{
    for (int i = 0; i < MAX_CLIENT_COUNT; i++)
    {
        if (!tcpClients[i])
        {
            LogTrace("TcpClient[%d] is null", i);
            continue;
        }

        LogTrace("TcpClient[%d] is %d", i, tcpClients[i]);

        Serial.print("+CIPSTATE:");
        Serial.print(i);
        Serial.print(",");
        Serial.print(tcpClients[i].remoteIP().toString());
        Serial.print(",");
        Serial.print(tcpClients[i].remotePort());
        Serial.print(",");
        Serial.println(tcpClients[i].localPort());
    }

    return AT_OK;
}

/**
 * @brief Sends the data to the client at specified channel.
 *
 * @param AT+CIPSEND=<link_ID>,<length>
 * @return  OK
 *          >
 */
char send_data(char *value)
{
    unsigned long len;
    unsigned int chan;

    sscanf(value, "%d,%lu", &chan, &len);

    if (len > sizeof(TCP_TX_BUFFER))
    {
        LogErr("Length of the data to send (%d) is greater than the buffer size (%d).", len, TCP_TX_BUFFER);
        return AT_ERROR;
    }

    if (chan >= tcpClients.size())
    {
        LogWarn("Specified chan %d is not present (%d chanels currently connected).", chan, tcpClients.size());
        return AT_ERROR;
    }

    LogTrace("Preparing to send %lu bytes to channel %d", len, chan);

    WiFiClient client = tcpClients[chan];

    if (!client.connected())
    {
        LogErr("Client is not connected.");
        return AT_ERROR;
    }

    stop_at_processing = true;

    Serial.println("OK");
    Serial.print("> ");

    unsigned long read = 0;

    while (read < len)
    {
        if (!Serial.available())
        {
            continue;
        }

        char byte = Serial.read();
        TCP_TX_BUFFER[read++] = byte;
    }

    LogTrace("Sending %lu bytes to channel %d\n%s", len, chan, TCP_TX_BUFFER);

    unsigned long sent = client.write(TCP_TX_BUFFER, len);

    if (sent != len)
    {
        LogErr("Failed to send %lu bytes to channel %d", len, chan);
        stop_at_processing = false;

        return AT_ERROR;
    }

    Serial.print("SEND OK");

    while (Serial.available())
        Serial.read();

    Serial.flush();

    stop_at_processing = false;

    return AT_OK;
}

/**
 * Registers the TCP/IP commands.
 *
 */
void register_tcp_ip_commands()
{
    at_register_command("CIPSERVER", (at_callback)get_server, (at_callback)set_server, 0, 0);
    at_register_command("CIPSTA", (at_callback)get_sta_ip_info, 0, 0, 0);
    at_register_command("CIPRECVLEN", (at_callback)get_server_data_len, 0, 0, 0);
    at_register_command("CIPRECVDATA", 0, (at_callback)get_server_data, 0, 0);
    at_register_command("CIPSTATE", (at_callback)get_connections_status, 0, 0, 0);
    at_register_command("CIPSEND", 0, (at_callback)send_data, 0, 0);
}
