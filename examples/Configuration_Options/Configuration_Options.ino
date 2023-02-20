/*
Vrekrer_scpi_parser library.
Configuration options example.

Demonstrates how to configure the library options

Vrekrer_scpi_parser library is designed for reduced RAM usage, needed for low 
RAM microcontrollers.
You can fine tune, or expand the default library capabilities by defining 
(before the library include)  the following macros:
SCPI_ARRAY_SYZE : Max branches of the command tree and max number of parameters.
SCPI_MAX_TOKENS : Max number of valid tokens.
SCPI_MAX_COMMANDS : Max number of registered commands.
SCPI_MAX_SPECIAL_COMMANDS : Max number of special commands.
SCPI_BUFFER_LENGTH : Length of the message buffer.
SCPI_HASH_TYPE : Integer size used for hashes.
*/

/*
As an example, the following command tree, including only the 
SCPI Required Commands and IEEE Mandated Commands 
will be analyzed and implemented with optimal RAM usage.

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
Largest branch needed = 3
i.e. STATus:OPERation:ENABle or SYSTem:ERRor:NEXT?
*/
#define SCPI_ARRAY_SYZE 3 //Default value = 6
//This also sets the max number of parameters

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
No special commands used
See RawData_Parameters example for further details.
*/
#define SCPI_MAX_SPECIAL_COMMANDS 0 //Default value = 0

/*
The message buffer should be large enough to fit all the incoming message
For example, the multicommand message
"*RST; *cls; status:operation:enable; status:questionable:enable;\n"
will need at least 67 byte buffer length.
*/
#define SCPI_BUFFER_LENGTH 128 //Default value = 64

/*
In order to reduce RAM usage, Vrekrer_scpi_parser library (ver. 0.42 and later)
uses a hash algorithm to store and compare registered commands. In very rare 
situations this might end in hash crashes (two commands have the same hash)

If needed, to avoid hash crashes, change SCPI_HASH_TYPE to uint16_t or uint32_t
*/
#define SCPI_HASH_TYPE uint8_t //Default value = uint8_t

#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

SCPI_Parser my_instrument;


void setup() {
  /*
  To fix hash crashes, the hashing magic numbers can be changed before 
  registering commands.
  Use prime numbers up to the SCPI_HASH_TYPE size.
  */
  my_instrument.hash_magic_number = 37; //Default value = 37
  my_instrument.hash_magic_offset = 7;  //Default value = 7

  /*
  Timeout time can be changed even during program execution
  See Error_Handling example for further details.
  */
  my_instrument.timeout = 10; //value in miliseconds. Default value = 10


  my_instrument.SetCommandTreeBase(F("STATus:OPERation"));
    my_instrument.RegisterCommand(F(":CONDition?"), &DoNothing);
    my_instrument.RegisterCommand(F(":ENABle"), &DoNothing);
    my_instrument.RegisterCommand(F(":EVENt?"), &DoNothing);
  my_instrument.SetCommandTreeBase(F("STATus:QUEStionable"));
    my_instrument.RegisterCommand(F(":CONDition?"), &DoNothing);
    my_instrument.RegisterCommand(F(":ENABle"), &DoNothing);
    my_instrument.RegisterCommand(F(":EVENt?"), &DoNothing);
  my_instrument.SetCommandTreeBase(F("STATus"));
    my_instrument.RegisterCommand(F(":OPERation?"), &DoNothing);
    my_instrument.RegisterCommand(F(":QUEStionable?"), &DoNothing);
    my_instrument.RegisterCommand(F(":PRESet"), &DoNothing);
  my_instrument.SetCommandTreeBase(F("SYSTem"));
    my_instrument.RegisterCommand(F(":ERRor?"), &DoNothing);
    my_instrument.RegisterCommand(F(":ERRor:NEXT?"), &DoNothing);
    my_instrument.RegisterCommand(F(":VERSion?"), &DoNothing);
  my_instrument.SetCommandTreeBase(F(""));
  my_instrument.RegisterCommand(F("*CLS"), &DoNothing);
  my_instrument.RegisterCommand(F("*ESE"), &DoNothing);
  my_instrument.RegisterCommand(F("*ESE?"), &DoNothing);
  my_instrument.RegisterCommand(F("*ESR"), &DoNothing);
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  my_instrument.RegisterCommand(F("*OPC"), &DoNothing);
  my_instrument.RegisterCommand(F("*OPC?"), &DoNothing);
  my_instrument.RegisterCommand(F("*RST"), &DoNothing);
  my_instrument.RegisterCommand(F("*SRE"), &DoNothing);
  my_instrument.RegisterCommand(F("*SRE?"), &DoNothing);
  my_instrument.RegisterCommand(F("*STB"), &DoNothing);
  my_instrument.RegisterCommand(F("*TST?"), &DoNothing);
  my_instrument.RegisterCommand(F("*WAI"), &DoNothing);

  Serial.begin(9600);
  while (!Serial) {;}
  
  /*
  void SCPI_Parser::PrintDebugInfo(Stream &interface)
  Will print any set up error to a Stream interface.
  Use it only to test your setup  (not needed for normal execution).
  */
  my_instrument.PrintDebugInfo(Serial);
}

void loop() {
  /*
  void SCPI_Parser::ProcessInput(Stream &interface, const char *term_chars)
  will process any message from a stream inteface.
  The used interface will be passed to the Registered command handlers.
  */
  my_instrument.ProcessInput(Serial, "\n");

  /*
  void SCPI_Parser::Execute(char *message, Stream &interface)
  can also be used to process a message.
  Use this if the message source is not an Stream.
  A stream like object is still needed for passing it to the command handlers.
  i.e.
    my_instrument.Execute(GetEthernetMsg(), Serial)   
  */
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(F("Vrekrer,Configuration options example,#00," 
                      VREKRER_SCPI_VERSION));
}

void DoNothing(SCPI_C commands, SCPI_P parameters, Stream& interface) {
}

void DoSpe(SCPI_C commands, Stream& interface) {
}

