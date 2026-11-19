/*
Vrekrer_scpi_parser library.
SCPI error handling example.

Demonstrates how to handle buffer overflows, timeouts while waiting for
a complete message, and unknown commands.

Hardware required: None

Commands:
  *IDN?
    Gets the instrument's identification string

  SYSTem:ERRor?
    Reads and clears the last error.
*/


#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

SCPI_Parser my_instrument;

void setup()
{
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  my_instrument.RegisterCommand(F("SYSTem:ERRor?"), &GetLastError);
  my_instrument.SetErrorHandler(&myErrorHandler);
  // The default error handler ignores errors.

  Serial.begin(9600);

  /*
  You can change the timeout while the program is running.
  */
  my_instrument.timeout = 10; // In milliseconds. Default value: 10.

}

void loop()
{
  my_instrument.ProcessInput(Serial, "\n");
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(
            F("Vrekrer,Error Handling Example,#00," VREKRER_SCPI_VERSION));
}

void GetLastError(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  switch(my_instrument.last_error){
    case my_instrument.ErrorCode::BufferOverflow: 
      interface.println(F("Buffer overflow error"));
      break;
    case my_instrument.ErrorCode::Timeout:
      interface.println(F("Communication timeout error"));
      break;
    case my_instrument.ErrorCode::UnknownCommand:
      interface.println(F("Unknown command received"));
      break;
    case my_instrument.ErrorCode::NoError:
      interface.println(F("No Error"));
      break;
  }
  my_instrument.last_error = my_instrument.ErrorCode::NoError;
}

void myErrorHandler(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // The parser calls this function when a communication error occurs.

  /* The error type is stored in my_instrument.last_error
     Possible errors are:
       SCPI_Parser::ErrorCode::NoError
       SCPI_Parser::ErrorCode::UnknownCommand
       SCPI_Parser::ErrorCode::Timeout
       SCPI_Parser::ErrorCode::BufferOverflow
  */

  /* After a buffer overflow, any bytes still arriving or waiting in the stream
     may cause another error. Read and discard those bytes here. */
  if (my_instrument.last_error == SCPI_Parser::ErrorCode::BufferOverflow) {
    delay(2);
    while (interface.available()) {
      delay(2);
      interface.read();
    }
  }

  /*
  For UnknownCommand errors, use commands and parameters to inspect the
  received message.
  */
}
