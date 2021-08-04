#ifndef VREKRER_SCPI_PARSER_H_
#define VREKRER_SCPI_PARSER_H_

/*!
Usage:
See the examples
*/

/*!
Definitions:

Termination chars.
 A string formed by any characters that indicates the end of a message.
 Usual termination chars are LineFeed '\n' and/or CarrierReturn '\r'.
 Example:
  "\n"
  "\r\n"

Message.
 A string that could contain one or more commands and parameters, and is
 ended by termination chars.
 Example:
  "*IDN?; SYSTEM:TIMER:SET 5, minutes; syst:led:brig:inc\n"
  "This message does not contain valid commands.\r\n"

Tokens.
 Keywords (strings) used to form a command.
 e.g.
  "SYSTEM", "syst", "*IDN", "TIMER", "SET"
 Tokens are not case-sensitive.
 Tokens usually have a long form (complete word) and a short form (partial word).
 To define valid tokens, we use uppercase for the short form and lowercase to
 complete the long form (if needed).
 Example:
  "SYSTem"
  Sort form: "SYST"
  Long form: "system"

Commands.
 One or more tokens separated by the ':' character.
 Example:
  "*IDN?", "system:timer:set"
 A command terminated in the '?' character is defined as a "query".
 Inside a message, commands are separated by the ';' character.

Parameters.
 Comma ',' separated strings that come after a command in a message.
 Example:
  For the message "SYSTEM:TIMER:SET 5, minutes" the parameters are:
  "5" and "minutes"
 Spaces between parameters are ignored.

Valid commands / Command tree.
 Set of valid commands, usually defined in a branched form.
 The ':' token separator defines the branches
 Example:
  "SYSTem:LED:BRIGhtness:INCrease" (Valid command with branch size 4)
  "SYSTem:LED:BRIGhtness:DEcrease"
  Both commands share the TreeBase "SYSTem:LED:BRIGhtness"
*/


/// Max branch size of the command tree and max number of parameters.
#ifndef SCPI_ARRAY_SYZE
  #define SCPI_ARRAY_SYZE 6
#endif

/// Max number of valid tokens.
#ifndef SCPI_MAX_TOKENS
  #define SCPI_MAX_TOKENS 15
#endif

/// Max number of registered commands.
#ifndef SCPI_MAX_COMMANDS
  #define SCPI_MAX_COMMANDS 20
#endif

/// Length of the message buffer.
#ifndef SCPI_BUFFER_LENGTH
  #define SCPI_BUFFER_LENGTH 64
#endif

/// Timeout, in miliseconds, for GetMessage and ProcessInput.
#ifndef SCPI_TIMEOUT
  #define SCPI_TIMEOUT 10
#endif

/// Integer size used for hashes.
#ifndef SCPI_HASH_TYPE
  #define SCPI_HASH_TYPE uint8_t
#endif

#include "Arduino.h"

/*!
 Variable size string array class.

 The array must be filled using the Append method (acts as a LIFO stack Push).
 Values can be extracted (and removed) using the Pop function (LIFO stack Pop).
 Both Append and Pop modifies the Size of the array.
 Values can be read (without removing them) using the following methods:
  First() : Returns the first value appended to the array.
  Last()  : Returns the last value appended to the array.
  Indexing (e.g. my_array[1] to get the second value of the array).
 The max size of the array is defined by SCPI_ARRAY_SYZE (default 6).
*/
class SCPI_String_Array {
 public:
  char* operator[](const byte index);  //Add indexing capability
  void Append(char* value);            //Append new string (LIFO stack Push)
  char* Pop();                         //LIFO stack Pop
  char* First();                       //Returns the first element of the array
  char* Last();                        //Returns the last element of the array
  uint8_t Size();                      //Array size
 protected:
  uint8_t size_ = 0;              //Internal array size
  char* values_[SCPI_ARRAY_SYZE]; //Storage of the strings
};

/*!
 String array class used to store the tokens of a command.

 This class inherits the properties of SCPI_String_Array.
 @see SCPI_String_Array
*/
class SCPI_Commands : public SCPI_String_Array {
 public:
  //Dummy constructor.
  SCPI_Commands();
  //Constructor that extracts and tokenize a command from a message
  SCPI_Commands(char* message);
  ///Not processed part of the message after the constructor is called.
  char* not_processed_message;
};

/*!
 String array class used to store the parameters found after a command.

 This class inherits the properties of SCPI_String_Array.
 @see SCPI_String_Array
*/
class SCPI_Parameters : public SCPI_String_Array {
 public:
  //Dummy constructor.
  SCPI_Parameters();
  //Constructor that extracts and splits parameters from a message
  SCPI_Parameters(char *message);
  ///Not processed part of the message after the constructor is called.
  char* not_processed_message;
};

typedef SCPI_Commands SCPI_C; ///Alias of SCPI_Commands.
typedef SCPI_Parameters SCPI_P; ///Alias of SCPI_Parameters.

///Void template used with SCPI_Parser::RegisterCommand.
typedef void (*SCPI_caller_t)(SCPI_Commands, SCPI_Parameters, Stream&);

/*!
  Main class of the Vrekrer_SCPI_Parser library.
*/
class SCPI_Parser {
 public:
  //Constructor
  SCPI_Parser();
  //Change the TreeBase for the next RegisterCommand calls
  void SetCommandTreeBase(char* tree_base);
  //SetCommandTreeBase version with RAM string support
  void SetCommandTreeBase(const char* tree_base);
  //SetCommandTreeBase version with Flash strings (F() macro) support
  void SetCommandTreeBase(const __FlashStringHelper* tree_base);
  //Registers a new valid command and associate a procedure to it
  void RegisterCommand(char* command, SCPI_caller_t caller);
  //RegisterCommand version with RAM string support.
  void RegisterCommand(const char* command, SCPI_caller_t caller);
  //RegisterCommand version with Flash strings (F() macro) support
  void RegisterCommand(const __FlashStringHelper* command, SCPI_caller_t caller);
  //Set the function to be used by the error handler.
  void SetErrorHandler(SCPI_caller_t caller);
  ///SCPI Error codes.
  enum class ErrorCode{
    NoError,
    UnknownCommand, ///Unknown command receiving
    Timeout, ///Timeout before receiving the termination chars
    BufferOverflow, ///Message buffer overflow
  };
  ///Variable that holds the last error code.
  ErrorCode last_error = ErrorCode::NoError;
  //Process a message and execute it a valid command is found
  void Execute(char* message, Stream& interface);
  //Gets a message from a Stream interface and execute it
  void ProcessInput(Stream &interface, const char* term_chars);
  //Gets a message from a Stream interface
  char* GetMessage(Stream& interface, const char* term_chars);
  //Prints registered tokens and command hashes to the serial interface
  void PrintDebugInfo();
  
 protected:
  //Add a token to the tokens' storage
  void AddToken_(char* token);
  //Get a hash from a command
  SCPI_HASH_TYPE GetCommandCode_(SCPI_Commands& commands);
  //Number of stored tokens
  uint8_t tokens_size_ = 0;
  //Storage for tokens
  char *tokens_[SCPI_MAX_TOKENS];
  //Number of registered commands
  uint8_t codes_size_ = 0;
  //Registered commands' hash storage
  SCPI_HASH_TYPE valid_codes_[SCPI_MAX_COMMANDS];
  //Pointers to the functions to be called when a valid command is received
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS+1];
  //Branch's hash used when calculating unique codes (0 for root)
  SCPI_HASH_TYPE tree_code_ = 0;
  //Message buffer.
  char msg_buffer_[SCPI_BUFFER_LENGTH];
  //Length of the readed message
  uint8_t message_length_ = 0;
  //Varible used for checking timeout errors
  unsigned long time_checker_;
};

#endif //VREKRER_SCPI_PARSER_H_
