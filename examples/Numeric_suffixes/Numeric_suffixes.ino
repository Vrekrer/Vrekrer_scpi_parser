/* 
Vrekrer_scpi_parser library.
Numeric suffixes example.

Demonstrates how to use commands with numeric suffixes to control digital
outputs and read digital inputs.

Digital Inputs:
 DIn0 = pin 8
 DIn1 = pin 9
 DIn2 = pin 10
 DIn3 = pin 11
 DIn4 = pin 12
 DIn5 = pin 13

Digital Outputs:
 DOut0 = pin 2
 DOut1 = pin 3
 DOut2 = pin 4
 DOut3 = pin 5
 DOut4 = pin 6
 DOut5 = pin 7

Commands:
  *IDN?
    Gets the instrument's identification string
 
 DIn<index>?
    Queries the logic state of DIn<index>
    
 DOut<index>? 
    Queries the logic state of DOut<index>
 
 DOut<index> HIGH|LOW|ON|OFF|1|0
   Sets the logic state of DOut<index>

See the code comments for further information and examples.

This example uses custom commands outside the standard SCPI command set.
*/


#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

//Digital input pins
const int DIn[6] = {8, 9, 10, 11, 12, 13};

//Digital output pins
const int DOut[6] = {2, 3, 4, 5, 6, 7};

SCPI_Parser my_instrument;

void setup()
{
  my_instrument.RegisterCommand("*IDN?", &Identify);

  //Use "#" at the end of a token to accept numeric suffixes.
  my_instrument.RegisterCommand(F("DIn#?"), &QueryDigital_Input);
  my_instrument.RegisterCommand(F("DOut#"), &WriteDigital_Output);
  my_instrument.RegisterCommand(F("DOut#?"), &QueryDigital_Output);

  for (int i = 0; i < 6; i++) {
    pinMode(DIn[i], INPUT);
  }
  for (int i = 0; i < 6; i++) {
    pinMode(DOut[i], OUTPUT);
  }

  Serial.begin(9600);
}

void loop()
{
  my_instrument.ProcessInput(Serial, "\n");
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    interface.println(F("Vrekrer,SCPI Numeric suffixes example,#00," 
                        VREKRER_SCPI_VERSION));
}

void QueryDigital_Input(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //DIn<index>?
  // Reads the logic state of the pin in DIn[index].
  //Return values are "HIGH" or "LOW"
  //Examples:
  // DIn0?     (Reads DIn[0].)
  // DI5?      (Reads DIn[5].)

  // Extract the pin index from the numeric suffix, if present.
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DIN]%u", &suffix);

  //If the suffix is valid, print the pin's logic state to the interface
  if ( (suffix >= 0) && (suffix < 6) ) {
    if (digitalRead(DIn[suffix]) == HIGH) {
      interface.println("HIGH");
    } else { //LOW
      interface.println("LOW");
    }
  }
}

void QueryDigital_Output(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //DOut<index>?
  // Reads the logic state of the pin in DOut[index].
  //Return values are "HIGH" or "LOW"
  //Examples:
  // DO4?      (Reads DOut[4].)
  // DOut6?    (Index 6 is outside the array.)

  // Extract the pin index from the numeric suffix, if present.
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DOUT]%u", &suffix);

  //If the suffix is valid, print the pin's logic value to the interface
  if ( (suffix >= 0) && (suffix < 6) ) {
    if (digitalRead(DOut[suffix]) == HIGH) {
      interface.println("HIGH");
    } else { //LOW
      interface.println("LOW");
    }
  }
}

void WriteDigital_Output(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //DOut<index> state
  // Sets the logic state of the pin in DOut[index].
  //Valid states are : "HIGH", "LOW", "ON", "OFF", "1" and "0"
  //and any lowercase/uppercase combinations
  //Examples:
  // DOut4 HIGH  (Sets DOut[4] to HIGH)
  // DO2 Off     (Sets DOut[2] to LOW)
  // DOUT0 1     (Sets DOut[0] to HIGH)

  // Extract the pin index from the numeric suffix, if present.
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DOUT]%u", &suffix);

  // Use the first parameter to set the output at the selected index.
  String first_parameter = String(parameters.First());
  first_parameter.toUpperCase();
  if ( (suffix >= 0) && (suffix < 6) ) {
    if ( (first_parameter == "HIGH")
       || (first_parameter == "ON")
       || (first_parameter == "1") ) {
      digitalWrite(DOut[suffix], HIGH);
    } else if ( (first_parameter == "LOW")
       || (first_parameter == "OFF")
       || (first_parameter == "0") ) {
      digitalWrite(DOut[suffix], LOW);
    }
  }
}
