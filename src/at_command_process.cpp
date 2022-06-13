
#include <Arduino.h>

#include "at_command_process.h"
#include "at_parser.h"
#include "logging.h"

#define BUFFER_SIZE 2048

bool stop_at_processing = false;

void process_at_commands()
{
  static String readString = "";
  char ret[BUFFER_SIZE];
  char res;

  if (stop_at_processing)
  {
    return;
  }

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
        LogErr("Input is too long");
        Serial.println();
        Serial.println(AT_ERROR_STRING);
        readString = "";
      }
      else
      {
        if (c == '\r' || c == ';')
        {
          readString.trim();

          if (!readString.startsWith("AT"))
          {
            readString = "";
          }
          else if (readString == "AT")
          {
            Serial.println();
            Serial.println(AT_OK_STRING);
            readString = "";
          }
          else
          {
            // Parsing the command
            res = at_parse_line(readString.c_str(), (unsigned char *)ret);

            readString = "";

            if (res == AT_OK)
            {
              if (ms_strlen(ret) > 0)
              {
                String s_ret(ret);
                Serial.println(s_ret);
              }
              Serial.println();
              Serial.println(AT_OK_STRING);
            }
            else
            {
              Serial.println();
              Serial.println(AT_ERROR_STRING);
            }
          }
        }
      }
    } // end serial available
  }   // end while
}