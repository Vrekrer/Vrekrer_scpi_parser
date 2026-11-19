/*
Vrekrer_scpi_parser library.
SCPI Special Commands example.

Demonstrates how to set up and use special commands.
Special command callbacks read and process message parameters directly
from the Stream. They can read text or raw data containing special characters,
handle data larger than the input buffer, and process it as it arrives.
*/

// Set SCPI_MAX_SPECIAL_COMMANDS to enable special commands.
// See the Configuration_Options example for details.
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
The normal callback runs once the parser receives a complete message.
Semicolons separate commands in the message. Colons separate tokens in each
command, and commas separate parameters. The parser trims leading whitespace
from each parameter. A timeout or buffer overflow prevents the callback from running.
*/
void NormalEcho(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // Print the command tokens.
  for (size_t i = 0; i < commands.Size(); i++) {
    interface.print(commands[i]);
    if (i < commands.Size()-1) {
      interface.print(":");  
    } else {
      interface.print(" ");
    }
  }
  
  // Print the received parameters.
  for (size_t i = 0; i < parameters.Size(); i++) {
    interface.print(parameters[i]);
    if (i < parameters.Size()-1)
      interface.print(", ");  
  }
  interface.println(); // Send a newline as the message terminator.
}


/*
The special callback runs when the parser reads the first space or tab after
the command header. It receives the command tokens and the Stream, then reads
the remaining bytes directly from the Stream while the main loop waits.
*/
void SpecialEcho(SCPI_C commands, Stream& interface) {
  // Print the command tokens, as in NormalEcho().
  for (size_t i = 0; i < commands.Size(); i++) {
    interface.print(commands[i]);
    if (i < commands.Size()-1) {
      interface.print(":");  
    } else {
      interface.print(" ");
    }
  }

  // Echo the remaining bytes to the stream.
  while (true) {
    if (interface.available()) {
      interface.print(char(interface.peek()));
      interface.flush();
      if (interface.read() == '\n') break; // Stop at the newline.
    }
  // This loop waits indefinitely for a newline. Handle missing or
  // unexpected data here in a real application.
  }
}
// A special command requires a space or tab after its header.
//"ECHO:SPEcial;" or "ECHO:SPEcial\n" will not call SpecialEcho.
