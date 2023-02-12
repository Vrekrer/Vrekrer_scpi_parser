/*
Vrekrer_scpi_parser library.
CommandTree_Check example.

Demonstrates how to verify the validity of the registered command tree.

In order to reduce RAM usage, Vrekrer_scpi_parser library (ver. 0.5 and later)
uses a hash algorithm to store and compare registered commands. In very rare 
situations this might end in hash crashes (two commands have the same hash).

To check the uniqueness of the registered commands' hashes the `PrintDebugInfo`
function must be used.
If a hash crash exists, this can be solved changing the `hash_magic_number` 
variable before registering the commands
*/


#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

SCPI_Parser my_instrument;

void setup()
{
  //We change the `hash_magic_number` variable before registering the commands
  my_instrument.hash_magic_number = 16; //16 will generate hash crashes
  //The default value is 37 and good values are prime numbers (up to 113)

  //This is a simple command tree with 8 registered commands:
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);   //*IDN?
  my_instrument.SetCommandTreeBase(F("TEST:"));
    my_instrument.RegisterCommand(F(":A"), &DoNothing);  //TEST:A
    my_instrument.RegisterCommand(F(":A?"), &DoNothing); //TEST:A?
    my_instrument.RegisterCommand(F(":B"), &DoNothing);  //TEST:B
    my_instrument.RegisterCommand(F(":C"), &DoNothing);  //TEST:C
  my_instrument.SetCommandTreeBase(F("NEW:TEST"));
    my_instrument.RegisterCommand(F(":A"), &DoNothing);  //NEW:TEST:A
  my_instrument.SetCommandTreeBase(F("OLD:TEST"));
    my_instrument.RegisterCommand(F(":B"), &DoNothing);  //OLD:TEST:B
    my_instrument.RegisterCommand(F(":C"), &DoNothing);  //OLD:TEST:C

  Serial.begin(9600);
  while (!Serial) {;}
  
  //`PrintDebugInfo` will print the registered tokens and 
  //command hashes to the serial interface.
  my_instrument.PrintDebugInfo();
  //See the result in the serial monitor and verify that
  //there are no duplicated hashes or hashes equal to zero.
  //Change the hash_magic_number to solve any problem.
}

void loop()
{
  my_instrument.ProcessInput(Serial, "\n");
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(F("Vrekrer,Hash check example,#00," VREKRER_SCPI_VERSION));
}

void DoNothing(SCPI_C commands, SCPI_P parameters, Stream& interface) {
}

