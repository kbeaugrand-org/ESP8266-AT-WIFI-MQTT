
#include <Arduino.h>
#include "at_parser.h"
#include "logging.h"

#include "common.h"
#include "tcp_ip_commands.h"

#include <ESP8266WiFi.h>

#define MAX_BUFFER_SIE 1024
#define MAX_CLIENT_COUNT 1

WiFiServer *tcpServer = nullptr;
WiFiClient **tcpClients = (WiFiClient **)malloc(sizeof(WiFiClient *) * MAX_CLIENT_COUNT);

bool tcpServerStarted = false;
bool connectedChannels[MAX_CLIENT_COUNT] = {};

char TCP_RX_BUFFER[MAX_CLIENT_COUNT][MAX_BUFFER_SIE] = {};
char TCP_TX_BUFFER[MAX_CLIENT_COUNT][MAX_BUFFER_SIE] = {};
int TCP_RX_BYTES[MAX_CLIENT_COUNT] = {};

/**
 * @brief Registers the WiFi Client channel for further processing.
 *
 * @param client
 * @return int representing the channel ID for the client.
 */
int register_client(WiFiClient *client)
{
    // Find corresponding client
    for (int i = 0; i < MAX_CLIENT_COUNT; i++)
    {
        if (!tcpClients[i])
        {
            continue;
        }

        // Server has already this client.
        // Re-affecting the channel
        if (tcpClients[i]->remoteIP() == client->remoteIP() && tcpClients[i]->remotePort() == client->remotePort())
        {
            LogTrace("Re-affecting client %d to %d", &client, i);
            tcpClients[i] = client;
            return i;
        }
    }

    for (int i = 0; i < MAX_CLIENT_COUNT; i++)
    {
        if (tcpClients[i])
        {
            continue;
        }

        LogTrace("Registering client %d to %d", &client, i);
        tcpClients[i] = client;
        return i;
    }

    LogWarn("Failed to register client %d", &client);

    return -1;
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
    for (int i = 0; i < MAX_CLIENT_COUNT; i++)
    {
        if (!tcpClients[i])
        {
            LogTrace("TcpClient[%d] is null", i);
            continue;
        }

        LogTrace("TcpClient[%d] is %d", i, tcpClients[i]);
        Serial.printf("+CIPRECVLEN:%d,%d\n", i, TCP_RX_BYTES[i]);
    }

    return AT_OK;
}

/**
 * @brief Obtain Socket Data in Passive Receiving Mode
 *
 * @param AT+CIPRECVDATA=<chan>,<len>
 */
char get_server_data(char *value)
{
    int len;
    int chan;
    sscanf(value, "%d,%d", &chan, &len);

    char buffer[len];
    memcpy(TCP_RX_BUFFER, buffer, sizeof(buffer));

    if (chan < 0 || chan > 9)
    {
        return AT_ERROR;
    }

    sprintf(value, "+CIPRECVDATA:%d,%s", TCP_RX_BYTES[chan], TCP_RX_BUFFER[chan]);
    TCP_RX_BYTES[chan] = 0;

    free(buffer);

    return AT_OK;
}

/**
 * @brief Process the TCP Clients
 *  it checks for each connected clients if some data are present and add it to the buffer.
 *
 */
void process_existing_channels()
{
    for (int channelID = 0; channelID < MAX_CLIENT_COUNT; channelID++)
    {
        if (!tcpClients[channelID])
        {
            continue;
        }

        if (!tcpClients[channelID]->connected())
        {
            LogTrace("Client is not connected.");
            continue;
        }

        int available = tcpClients[channelID]->available();

        if (!available)
        {
            LogInfo("Client connected without posting data.");
            continue;
        }

        char buffer[available];
        int bytes = tcpClients[channelID]->readBytes(buffer, available <= 1024 ? available : 1024);

        memcpy(buffer, TCP_RX_BUFFER[channelID] + TCP_RX_BYTES[channelID], bytes);

        TCP_RX_BYTES[channelID] += bytes;
        LogTrace("Got %d bytes on channel %d - Now %d bytes are waiting.", bytes, channelID, TCP_RX_BYTES[channelID]);

        Serial.print("+CIPRECVLEN:");
        Serial.println(channelID);
        Serial.print(",");
        Serial.println(TCP_RX_BYTES[channelID]);
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

    int channelID = register_client(&client);

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
        Serial.print(tcpClients[i]->remoteIP().toString());
        Serial.print(",");
        Serial.print(tcpClients[i]->remotePort());
        Serial.print(",");
        Serial.println(tcpClients[i]->localPort());
    }

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
}
