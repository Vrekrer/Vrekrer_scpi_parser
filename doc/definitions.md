## Termination chars.
 A string formed by any characters that indicates the end of a message.  
 Usual termination chars are LineFeed ``'\n'`` and/or CarrierReturn  ``'\r'``.  
 **Examples:**  
  ``"\n"``, ``"\r\n"``

## Message.
 A string that could contain one or more commands and parameters, and is
 ended by termination chars.  
 **Examples:**  
  ``"*IDN?; SYSTEM:TIMER:SET 5, minutes; syst:led:brig:inc\n"``  
  ``"This message does not contain valid commands.\r\n"``

## Tokens.
 Keywords (strings) used to form a command.  
 **Examples:**  
  ``"SYSTEM"``, ``"syst"``, ``"*IDN"``, ``"TIMER"``, ``"SET"``  

 Tokens are not case-sensitive.  
 Tokens usually have a long form (complete word) and a short form (partial word).
 
 To define valid tokens, we use uppercase for the short form and lowercase to
 complete the long form (if needed).  
 **Example:**  
  ``"SYSTem"``; sort form: ``"SYST"``, long form: ``"system"``  

## Commands.
 One or more tokens separated by the `':'` character.  
 Examples:  
 ``"*IDN?"``,  ``"system:timer:set"``  
 A command terminated in the ``'?'`` character is defined as a "query".  
 Inside a message, commands are separated by the ``';'`` character.

## Parameters.
 Comma ``','`` separated strings that come after a command in a message.  
 **Example:**  
  For the message ``"SYSTEM:TIMER:SET 5, minutes"`` 
  the parameters are: ``"5"`` and ``"minutes"``  
  Spaces between parameters are ignored.

## Valid commands / Command tree.
 Set of valid commands, usually defined in a branched form.  
 The ``':'`` token separator defines the branches.  
 **Example:**  
  ``"SYSTem:LED:BRIGhtness:INCrease"`` (Valid command with branch size 4)  
  ``"SYSTem:LED:BRIGhtness:DEcrease"``  
  Both commands share the TreeBase ``"SYSTem:LED:BRIGhtness"``

