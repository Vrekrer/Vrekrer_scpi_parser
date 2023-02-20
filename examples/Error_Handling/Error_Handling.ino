/*
Vrekrer_scpi_parser library.
SCPI Errors Handling example.

Demonstrate how to handle communication errors.
These include : 
  Message buffer overflow,
  Timeout before receiving the termination chars (uncompleted messages)
  and receiving an unknown command.

Hardware required: None

Commands:
  *IDN?
    Gets the instrument's identification string

  SYSTem:ERRor?
    Reads the last error occurred and then delete it.
*/


#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

SCPI_Parser my_instrument;

void setup()
{
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  my_instrument.RegisterCommand(F("SYSTem:ERRor?"), &GetLastEror);
  my_instrument.SetErrorHandler(&myErrorHandler);
  //Not setting an error handler will just ignore the errors.

  Serial.begin(9600);

  /*
  Timeout time can be changed even during program execution
  */
  my_instrument.timeout = 10; //value in miliseconds. Default value = 10

}

void loop()
{
  my_instrument.ProcessInput(Serial, "\n");
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(
            F("Vrekrer,Error Handling Example,#00," VREKRER_SCPI_VERSION));
}

void GetLastEror(SCPI_C commands, SCPI_P parameters, Stream& interface) {
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
  //This function is called every time an error occurs

  /* The error type is stored in my_instrument.last_error
     Possible errors are:
       SCPI_Parser::ErrorCode::NoError
       SCPI_Parser::ErrorCode::UnknownCommand
       SCPI_Parser::ErrorCode::Timeout
       SCPI_Parser::ErrorCode::BufferOverflow
  */

  /* For BufferOverflow errors, the rest of the message, still in the interface
  buffer or not yet received, will be processed later and probably 
  trigger another kind of error.
  Here we flush the incomming message*/
  if (my_instrument.last_error == SCPI_Parser::ErrorCode::BufferOverflow) {
    delay(2);
    while (interface.available()) {
      delay(2);
      interface.read();
    }
  }

  /*
  For UnknownCommand errors, you can get the received unknown command and
  parameters from the commands and parameters variables.
  */
}
