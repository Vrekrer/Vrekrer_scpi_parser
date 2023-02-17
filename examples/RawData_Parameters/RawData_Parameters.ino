/*
Vrekrer_scpi_parser library.
SCPI Special Commands example.

Demonstrates how to set up and use special commands.
Special commands allow the user to write custom code for reading and
processing the message parameters.

This can be used for:
 - Read Strings or raw data that contain special characters.
 - Read data larger than the input buffer.
 - Real time processing of the data.
 - etc.
*/

//For using special commands, SCPI_MAX_SPECIAL_COMMANDS must be defined.
//See the Configuration_Options example for further information.
//We will use only one special command here.
#define SCPI_MAX_SPECIAL_COMMANDS 1  //default 0

#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

SCPI_Parser my_instrument;
void setup()
{
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  my_instrument.RegisterCommand(F("ECHO:NORMal"), &NormalEcho);
  my_instrument.RegisterCommand(F("ECHO:NORMal?"), &NormalEcho);
  my_instrument.RegisterSpecialCommand(F("ECHO:SPEcial"), &SpecialEcho);
  
  Serial.begin(9600);
}

void loop()
{
  my_instrument.ProcessInput(Serial, "\n");
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(F("Vrekrer,SCPI Special Commands Example,#00," 
                      VREKRER_SCPI_VERSION));
}

/*
## Normal handler definition. ##
This procedure will be called when the terminal chars (here '\n' line feed) or
the multicommand (';') char are received.
The main loop will not be stopped until all the message is received.
The received commands will be split using ':'.
The received parameters will be split using ',' and trailing spaces will
be ignored.
If a timeout or buffer overflow error happens, the entire received message will
be discarded, and this procedure will not be called.
*/
void NormalEcho(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //Here we print the recieved commands.
  for (size_t i = 0; i < commands.Size(); i++) {
    interface.print(commands[i]);
    if (i < commands.Size()-1) {
      interface.print(":");  
    } else {
      interface.print(" ");
    }
  }
  
  //Here we print the recieved parameters.
  for (size_t i = 0; i < parameters.Size(); i++) {
    interface.print(parameters[i]);
    if (i < parameters.Size()-1)
      interface.print(", ");  
  }
  interface.println(); // Send '\n' as terminal char.
}


/*
## Special handler definition. ##
This procedure will be called when the first ' ' (space) is received.
This only includes the command string.
The main loop is stopped without reading the parameters.
-Note- The void template include only commands an interface.
*/
void SpecialEcho(SCPI_C commands, Stream& interface) {
  //Same as in NormalEcho, here we print the recieved commands.
  for (size_t i = 0; i < commands.Size(); i++) {
    interface.print(commands[i]);
    if (i < commands.Size()-1) {
      interface.print(":");  
    } else {
      interface.print(" ");
    }
  }

  //Here we print back to the interface all the recieved chars
  while (true) {
    if (interface.available()) {
      interface.print(char(interface.peek()));
      interface.flush();
      if (interface.read() == '\n') break; //End if a '\n' is recieved
    }
  //Caution!!
  //Infinite loop
  //Any unexpected or missing data should be handled here.
  }
}
//A space ' ' after the command string is needed to trigger an special command.
//"ECHO:SPEcial;" or "ECHO:SPEcial\n" will not call SpecialEcho.
