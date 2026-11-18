# Vrekrer scpi parser
A simple [SCPI](https://en.wikipedia.org/wiki/Standard_Commands_for_Programmable_Instruments) parser for Arduino projects.

## Features
- Small memory footprint, implemented for reduced RAM usage.
- Minimal, clean code API.
- Can process char* strings or input from any [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/) interface like [Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial) or [Ethernet](https://www.arduino.cc/en/Reference/Ethernet).
- Flash strings ([F() macro](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/#_the_f_macro)) support for lower RAM usage.
- Automatic `Stream` communication errors handling (timeout, buffer overflow)


## SCPI features:
 - Commands defined using stings  
   E.g. `"MEASure:VOLTage:DC?"`
 - Short and long token forms, and upper and lower case commands  
   E.g. `"MEASURE:VOLTAGE:DC?"`, `"meas:VoLt:DC?"`
 - Numeric suffixes using the `#` character:  
   E.g. definition : `"CHANnel#:SELect"`  
   E.g. usage : `"CHAN0:SEL"`, `"chan5:sel"`, `"chan13:sel"`
 - Comma separated parameters recognition.
 - Parameters treated as text, processed by the user program.
 - Option to process large raw data parameters.

## Checking the setup
Errors found while registering commands can be checked once all the
commands are registered:
 - Storage overflows (see the `SCPI_MAX_...` options).
 - Two commands with the same hash (only the first one can be executed).
 - Two tokens that accept the same short or long form, e.g. `STATus` and
   `STATe` (`STAT` always selects the first one).

```cpp
if (my_instrument.HasSetupError()) {
  my_instrument.PrintDebugInfo(Serial);
}
```
`GetSetupErrors()` returns the individual error flags.  
Commands are stored as hashes of `SCPI_HASH_TYPE` size (`uint8_t` by
default). If two commands get the same hash, use `uint16_t` or `uint32_t`.  
See the Configuration_Options example for details.