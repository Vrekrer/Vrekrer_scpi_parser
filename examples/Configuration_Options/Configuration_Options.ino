/*
Vrekrer_scpi_parser library.
Configuration options example.

Demonstrates how to configure the library.

The library is designed for microcontrollers with limited RAM.
Define the following macros before including the library header to adjust
its capacity and memory usage.

SCPI_ARRAY_SIZE : Maximum number of keywords in a command path or parameters
                  in a message.
SCPI_MAX_TOKENS : Max number of valid tokens.
SCPI_MAX_COMMANDS : Max number of registered commands.
SCPI_MAX_SPECIAL_COMMANDS : Max number of special commands.
SCPI_BUFFER_LENGTH : Length of the message buffer.
SCPI_HASH_TYPE : Integer size used for hashes.
*/

/*
This example uses the following tree of required SCPI and IEEE commands
to illustrate how to choose library settings.

STATus
    :OPERation
        :CONDition?
        :ENABle
        [:EVENt]?
    :QUEStionable
        :CONDition?
        :ENABle
        [:EVENt]?
    :PRESet
SYSTem
    :ERRor
        [:NEXT]?
    :VERSion?
*CLS
*ESE
*ESE?
*ESR?
*IDN?
*OPC
*OPC?
*RST
*SRE
*SRE?
*STB
*TST?
*WAI
*/

/*
The longest command paths contain three keywords.
Examples include STATus:OPERation:ENABle and SYSTem:ERRor:NEXT?.
*/
#define SCPI_ARRAY_SIZE 3 // Default value: 6
// This also sets the maximum number of parameters.

/*
Valid Tokens: 
01: STATus
02: OPERation
03: CONDition
04: ENABle
05: EVENt
06: QUEStionable
07: PRESet
08: SYSTem
09: ERRor
10: NEXt
11: VERSion
12: *CLS
13: *ESE
14: *ESR
15: *IDN
16: *OPC
17: *RST
18: *SRE
19: *STB
20: *TST
21: *WAI
Total number of valid tokens: 21
*/
#define SCPI_MAX_TOKENS 21 //Default value = 15

/*
Valid Commands:
01: STATus:OPERation:CONDition?
02: STATus:OPERation:ENABle
03: STATus:OPERation?
04: STATus:OPERation:EVENt?
05: STATus:QUEStionable:CONDition?
06: STATus:QUEStionable:ENABle
07: STATus:QUEStionable?
08: STATus:QUEStionable:EVENt?
09: STATus:PRESet
10: SYSTem:ERRor?
11: SYSTem:ERRor:NEXT?
12: SYSTem:VERSion?
13: *CLS
14: *ESE
15: *ESE?
16: *ESR?
17: *IDN?
18: *OPC
19: *OPC?
20: *RST
21: *SRE
22: *SRE?
23: *STB
24: *TST?
25: *WAI
Total number of valid commands: 25
*/
#define SCPI_MAX_COMMANDS 25 //Default value = 20

/*
This example uses no special commands.
See the RawData_Parameters example for details.
*/
#define SCPI_MAX_SPECIAL_COMMANDS 0 //Default value = 0

/*
The buffer must hold the entire incoming message.
For example, this message contains several commands
"*RST; *cls; status:operation:enable; status:questionable:enable;\n"
and needs a buffer of at least 67 bytes.
*/
#define SCPI_BUFFER_LENGTH 128 //Default value = 64

/*
Since version 0.42, the library has stored command hashes to reduce RAM usage.
A collision occurs when two commands have the same hash.

Use uint16_t or uint32_t for SCPI_HASH_TYPE to reduce the chance of collisions.
*/
#define SCPI_HASH_TYPE uint8_t //Default value = uint8_t

#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

SCPI_Parser my_instrument;


void setup() {
  /*
  You can change the hash constants before registering commands to resolve
  collisions. Use prime numbers that fit in SCPI_HASH_TYPE.
  */
  my_instrument.hash_magic_number = 37; //Default value = 37
  my_instrument.hash_magic_offset = 7;  //Default value = 7

  /*
  You can change the timeout while the program is running.
  See the Error_Handling example for details.
  */
  my_instrument.timeout = 10; // In milliseconds. Default value: 10.


  my_instrument.SetCommandTreeBase(F("STATus:OPERation"));
    my_instrument.RegisterCommand(F(":CONDition?"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":ENABle"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":EVENt?"), &IgnoreCommand);
  my_instrument.SetCommandTreeBase(F("STATus:QUEStionable"));
    my_instrument.RegisterCommand(F(":CONDition?"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":ENABle"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":EVENt?"), &IgnoreCommand);
  my_instrument.SetCommandTreeBase(F("STATus"));
    my_instrument.RegisterCommand(F(":OPERation?"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":QUEStionable?"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":PRESet"), &IgnoreCommand);
  my_instrument.SetCommandTreeBase(F("SYSTem"));
    my_instrument.RegisterCommand(F(":ERRor?"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":ERRor:NEXT?"), &IgnoreCommand);
    my_instrument.RegisterCommand(F(":VERSion?"), &IgnoreCommand);
  my_instrument.SetCommandTreeBase(F(""));
  my_instrument.RegisterCommand(F("*CLS"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*ESE"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*ESE?"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*ESR"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  my_instrument.RegisterCommand(F("*OPC"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*OPC?"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*RST"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*SRE"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*SRE?"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*STB"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*TST?"), &IgnoreCommand);
  my_instrument.RegisterCommand(F("*WAI"), &IgnoreCommand);

  Serial.begin(9600);
  while (!Serial) {;}
  
  /*
  void SCPI_Parser::PrintDebugInfo(Stream &interface)
  Prints setup errors to a Stream. Use it to check your configuration.
  */
  my_instrument.PrintDebugInfo(Serial);
}

void loop() {
  /*
  void SCPI_Parser::ProcessInput(Stream &interface, const char *term_chars)
  reads messages from a Stream and passes that stream to the command callbacks.
  */
  my_instrument.ProcessInput(Serial, "\n");

  /*
  void SCPI_Parser::Execute(char *message, Stream &interface)
  can also be used to process a message.
  Use this when the message comes from another source.
  The command callbacks still receive a Stream.
  i.e.
    my_instrument.Execute(GetEthernetMsg(), Serial)   
  */
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(F("Vrekrer,Configuration options example,#00," 
                      VREKRER_SCPI_VERSION));
}

void IgnoreCommand(SCPI_C commands, SCPI_P parameters, Stream& interface) {
}

void DoSpe(SCPI_C commands, Stream& interface) {
}

