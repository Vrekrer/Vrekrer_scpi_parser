# Vrekrer scpi parser
A simple [SCPI](https://en.wikipedia.org/wiki/Standard_Commands_for_Programmable_Instruments) parser for small Arduino projects.

## Features
- Small memory footprint, implemented for reduced RAM usage.
- Minimal, clean code API.
- Can process char* strings or input from any [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/) interface like [Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial) or [Ethernet](https://www.arduino.cc/en/Reference/Ethernet).
- Flash strings ([F() macro](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/#_the_f_macro)) support for lower RAM usage.


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

