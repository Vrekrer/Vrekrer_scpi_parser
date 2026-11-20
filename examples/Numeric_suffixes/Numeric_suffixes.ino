/* 
Vrekrer_scpi_parser library.
Numeric suffixes example.

Demonstrates how to use commands with numeric suffixes to control digital
outputs and read digital inputs and analog voltages.

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

Analog Inputs (Arduino Uno)
 Channel 1 = A0
 Channel 2 = A1

Commands:
 *IDN?
    Gets the instrument's identification string

 MEASure[<channel>]:VOLTage:DC?
   Reads the selected analog input in volts. The default channel is 1.
   MEAS:VOLT:DC? reads A0. MEAS2:VOLT:DC? reads A1.
 
 DIn<index>?
    Queries the logic state of DIn<index>
    
 DOut<index>? 
    Queries the logic state of DOut<index>
 
 DOut<index> HIGH|LOW|ON|OFF|1|0
   Sets the logic state of DOut<index>

See the code comments for further information and examples.

The digital input and output commands are custom commands.
*/


#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

//Digital input pins
const int DIn[6] = {8, 9, 10, 11, 12, 13};

//Digital output pins
const int DOut[6] = {2, 3, 4, 5, 6, 7};

//Analog input pins
const int AIn[2] = {A0, A1};
// Arduino Uno ADC scaling with a 5 V reference and 10-bit resolution.
const float volts_per_count = 5.0f / 1023.0f;

SCPI_Parser my_instrument;

void setup()
{
  my_instrument.RegisterCommand("*IDN?", &Identify);

  // Use '#' at the end of a token to accept numeric suffixes.
  my_instrument.RegisterCommand(F("MEASure#:VOLTage:DC?"), &MeasureAnalogVoltage);
  my_instrument.RegisterCommand(F("DIn#?"), &QueryDigital_Input);
  my_instrument.RegisterCommand(F("DOut#"), &WriteDigital_Output);
  my_instrument.RegisterCommand(F("DOut#?"), &QueryDigital_Output);
  // When a command is registered with '#' to accept numeric suffixes,
  // the parser processes commands with or without a numeric suffix
  // and calls the same handler in either case.
  // The handler can read the suffix from the corresponding keyword in
  // the SCPI_C commands argument, as shown in the examples below.
  //
  // Avoid registering the same command again without '#'.
  // Both definitions can match a command with no suffix,
  // so the order of registration can determine which handler runs.
  //
  // SCPI specifies that a missing numeric suffix defaults to 1.
  // The command handler must implement this behavior to follow SCPI.

  for (int i = 0; i < 6; i++) {
    pinMode(DIn[i], INPUT);
  }
  for (int i = 0; i < 6; i++) {
    pinMode(DOut[i], OUTPUT);
  }
  for (int i = 0; i < 2; i++) {
    pinMode(AIn[i], INPUT);
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

void MeasureAnalogVoltage(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // MEASure[<channel>]:VOLTage:DC?
  // Reads the voltage at the corresponding channel. The default channel is 1.
  //Examples:
  // MEAS:VOLT:DC?    reads Channel 1. (using default channel)
  // MEAS1:VOLT:DC?   reads Channel 1.
  // MEAS2:VOLT:DC?   reads Channel 2.

  // Extract the channel index from the numeric suffix, if present.
  String header = String(commands.First());
  header.toUpperCase();
  unsigned int suffix = 1; //Default value
  sscanf(header.c_str(),"%*[MEASUR]%u", &suffix);

  // If the suffix is valid, print the selected channel's voltage.
  if ( (suffix >= 1) && (suffix <= 2) ) {
    const int reading = analogRead(AIn[suffix - 1]);
    interface.println(reading * volts_per_count, 3);
  }
}

void QueryDigital_Input(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // DIn<index>?
  // Reads a digital input and returns "HIGH" or "LOW".
  // Examples
  // DIn0?   reads DIn[0].
  // DI5?    reads DIn[5].

  // Extract the pin index from the suffix. A missing suffix leaves -1.
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DIN]%u", &suffix);

  // Digital pin indices run from 0 to 5.
  if ( (suffix >= 0) && (suffix < 6) ) {
    if (digitalRead(DIn[suffix]) == HIGH) {
      interface.println("HIGH");
    } else {
      interface.println("LOW");
    }
  }
}

void QueryDigital_Output(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // DOut<index>?
  // Reads a digital output and returns "HIGH" or "LOW".
  // Examples
  // DO4?     reads DOut[4].
  // DOut6?   reads nothing as the index is outside the valid range.

  // Extract the pin index from the suffix. A missing suffix leaves -1.
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DOUT]%u", &suffix);

  // Digital pin indices run from 0 to 5.
  if ( (suffix >= 0) && (suffix < 6) ) {
    if (digitalRead(DOut[suffix]) == HIGH) {
      interface.println("HIGH");
    } else {
      interface.println("LOW");
    }
  }
}

void WriteDigital_Output(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // DOut<index> HIGH|LOW|ON|OFF|1|0
  // Sets a digital output. The state parameter is case-insensitive.
  // Examples
  // DOut4 HIGH   sets DOut[4] to HIGH.
  // DO2 Off      sets DOut[2] to LOW.
  // DOUT0 1      sets DOut[0] to HIGH.

  // Extract the pin index from the suffix. A missing suffix leaves -1.
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DOUT]%u", &suffix);

  // Read the requested state from the first parameter.
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
