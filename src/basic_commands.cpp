
#include <Arduino.h>
#include "at_parser.h"

#include "common.h"
#include "basic_commands.h"

void reset() {
    Serial.println("OK");
    ESP.restart();
}

char check_version_information(char *value) {

    Serial.println("AT version:" + (String)AT_VERSION);
    Serial.println("Bin version:" + (String)FIRMWARE_VERSION);
    return AT_OK;
}

char list_all_commands(char *value) {
    int i;

    for(i = 0; i < AT_COMMANDS_NUM; i++)
    {
        if(at_registered_commands[i].hash != 0)
        {
            AT_COMMAND current = at_registered_commands[i];

            Serial.print("+CMD:");
            Serial.print(i);
            Serial.print(",");
            Serial.print((const char*)current.name);
            Serial.print(",");
            Serial.print(current.test != NULL ? "1" : "0");
            Serial.print(",");
            Serial.print(current.getter != NULL ? "1" : "0");
            Serial.print(",");
            Serial.print(current.setter != NULL ? "1" : "0");
            Serial.print(",");
            Serial.print(current.execute != NULL ? "1" : "0");
            Serial.println();
        }
    }

    return AT_OK;
}

void register_basic_commands()
{
    at_register_command("RST", 0, 0, 0, (at_callback)reset);
    at_register_command("GMR", 0, 0, 0, (at_callback)check_version_information);
    at_register_command("CMD", (at_callback)list_all_commands, 0, 0, 0);
}
