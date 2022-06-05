
#include <Arduino.h>

#include "at_command_process.h"
#include "at_parser.h"

void process_at_commands()
{
  static String readString = "";
  char ret[50];
  char res;

  while (Serial.available())
  {
    if (Serial.available() > 0)
    {
      // Get a byte from buffer
      char c = Serial.read();

      readString += c;

      // Input is too long
      if (readString.length() > AT_MAX_TEMP_STRING)
      {
        Serial.println(AT_ERROR_STRING);
        readString = "";
      }
      else
      {
        if (c == '\r' || c == ';')
        {
          readString.trim();

          // Simple echo
          if (readString == "AT")
          {
            Serial.println(AT_OK_STRING);
            readString = "";
          }
          else
          {
            // Parsing the command
            res = at_parse_line((string_t)readString.c_str(), (unsigned char*)ret);

            readString = "";

            if (res == AT_OK)
            {
              if (ms_strlen((string_t)ret) > 0)
              {
                String s_ret(ret);
                Serial.println(s_ret);
              }
              Serial.println(AT_OK_STRING);
            }
            else
            {
              Serial.println(AT_ERROR_STRING);
            }
          }
        }
      }
    } // end serial available
  } // end while
}