/*! 
@file Vrekrer_scpi_parser.h 
Header file.
*/

#ifndef VREKRER_SCPI_PARSER_H_
#define VREKRER_SCPI_PARSER_H_

#define VREKRER_SCPI_VERSION "v0.5.0"


/// Maximum number of keywords in a command path and parameters in a message.
#ifndef SCPI_ARRAY_SIZE
  #define SCPI_ARRAY_SIZE 6
#endif

/// Max number of valid tokens.
#ifndef SCPI_MAX_TOKENS
  #define SCPI_MAX_TOKENS 15
#endif

/// Max number of registered commands.
#ifndef SCPI_MAX_COMMANDS
  #define SCPI_MAX_COMMANDS 20
#endif

/// Max number of registered special commands.
#ifndef SCPI_MAX_SPECIAL_COMMANDS
  #define SCPI_MAX_SPECIAL_COMMANDS 0
#endif

/// Length of the message buffer.
#ifndef SCPI_BUFFER_LENGTH
  #define SCPI_BUFFER_LENGTH 64
#endif

/// Integer size used for hashes.
#ifndef SCPI_HASH_TYPE
  #define SCPI_HASH_TYPE uint8_t
#endif

#include "Arduino.h"

/*!
 Array of strings with a variable length and fixed capacity.

 Use \c Append to add a value to the end of the array.
 Use \c Pop to retrieve and remove the last value.
 Both \c Append and \c Pop change the number of stored values.
 These operations read values without removing them.
  \li \c First() returns the first value.
  \li \c Last() returns the last value.
  \li Indexing reads a value by position. For example, \c my_array[1] reads
      the second value.

 \c SCPI_ARRAY_SIZE sets the array capacity (6 by default).
*/
class SCPI_String_Array {
 public:
  char* operator[](const byte index) const;  //Add indexing capability
  void Append(char* value);            //Append new string (LIFO stack Push)
  char* Pop();                         //LIFO stack Pop
  char* First() const;                       //Returns the first element of the array
  char* Last() const;                        //Returns the last element of the array
  uint8_t Size() const;                      //Array size
  bool overflow_error = false;         //Storage overflow error
  const uint8_t storage_size = SCPI_ARRAY_SIZE; // Array capacity.
 protected:
  uint8_t size_ = 0;              //Internal array size
  char* values_[SCPI_ARRAY_SIZE]; //Storage of the strings
};

/*!
 String array class used to store the tokens of a command.
 @see SCPI_String_Array
*/
class SCPI_Commands : public SCPI_String_Array {
 public:
  // Default constructor.
  SCPI_Commands();
  // Extracts and splits the command header into tokens.
  SCPI_Commands(char* message);
  /// Unprocessed portion of the message.
  char* not_processed_message;
};

/*!
 String array class used to store the parameters found after a command.
 @see SCPI_String_Array
*/
class SCPI_Parameters : public SCPI_String_Array {
 public:
  // Default constructor.
  SCPI_Parameters();
  // Extracts and splits the message parameters.
  SCPI_Parameters(char *message);
  /// Unprocessed portion of the message.
  char* not_processed_message;
};

///Alias of SCPI_Commands.
using SCPI_C = SCPI_Commands;

///Alias of SCPI_Parameters.
using SCPI_P = SCPI_Parameters;

/// Function pointer for command callbacks.
using SCPI_caller_t = void(*)(SCPI_Commands, SCPI_Parameters, Stream&);
/// Function pointer for special command callbacks.
using SCPI_special_caller_t = void(*)(SCPI_Commands, Stream&);

/// Integer size used for hashes.
using scpi_hash_t = SCPI_HASH_TYPE;

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
  // Registers a command and associates a callback with it.
  void RegisterCommand(char* command, SCPI_caller_t caller);
  //RegisterCommand version with RAM string support.
  void RegisterCommand(const char* command, SCPI_caller_t caller);
  //RegisterCommand version with Flash strings (F() macro) support
  void RegisterCommand(const __FlashStringHelper* command,
                       SCPI_caller_t caller);
  //Set the function to be used by the error handler.
  void SetErrorHandler(SCPI_caller_t caller);
  ///SCPI Error codes.
  enum class ErrorCode{
    ///No error
    NoError,
    ///Unknown command received.
    UnknownCommand,
    ///Timeout before receiving the termination chars.
    Timeout,
    ///Message buffer overflow.
    BufferOverflow,
  };
  ///Variable that holds the last error code.
  ErrorCode last_error = ErrorCode::NoError;
  // Processes a message and executes any matching commands.
  void Execute(char* message, Stream& interface);
  // Reads a message from a Stream and executes any matching commands.
  void ProcessInput(Stream& interface, const char* term_chars);
  //Gets a message from a Stream interface
  char* GetMessage(Stream& interface, const char* term_chars);
  // Prints registered tokens and command hashes to a Stream.
  void PrintDebugInfo(Stream& interface);
  ///Magic number used for hashing the commands
  scpi_hash_t hash_magic_number = 37;
  ///Magic offset used for hashing the commands
  scpi_hash_t hash_magic_offset = 7;
  // Timeout in milliseconds for GetMessage() and ProcessInput().
  unsigned long timeout = 10;
  
  #if SCPI_MAX_SPECIAL_COMMANDS
  // Registers a special command and associates a callback with it.
  void RegisterSpecialCommand(char* command, SCPI_special_caller_t caller);
  //RegisterSpecialCommand version with RAM string support.
  void RegisterSpecialCommand(const char* command, 
                              SCPI_special_caller_t caller);
  //RegisterSpecialCommand version with Flash strings (F() macro) support
  void RegisterSpecialCommand(const __FlashStringHelper* command, 
                              SCPI_special_caller_t caller);
  #endif

 protected:
  //Length of the message buffer.
  const uint8_t buffer_length = SCPI_BUFFER_LENGTH;
  //Max number of valid tokens.
  const uint8_t max_tokens = SCPI_MAX_TOKENS;
  //Max number of registered commands.
  const uint8_t max_commands = SCPI_MAX_COMMANDS;
  //Internal errors container
  struct internal_errors {
    //Command storage overflow error
    bool command_overflow = false;
    //Token storage overflow error
    bool token_overflow = false;
    //Branch (SCPI_Commands) storage overflow error
    bool branch_overflow = false;
    //Special command storage overflow error
    bool special_command_overflow = false;
  } setup_errors;
  //Hash result for unknown commands
  const scpi_hash_t unknown_hash = 0;
  //Hash reserved for invalid commands
  const scpi_hash_t invalid_hash = 1;

  //Add a token to the tokens' storage
  void AddToken_(char* token);
  //Get a hash from a command
  scpi_hash_t GetCommandCode_(SCPI_Commands& commands);
  struct CommandCode {
    scpi_hash_t code;
    bool invalid;
  };
  // Registers the tokens and checks the command before storing its callback.
  CommandCode PrepareCommand_(char* command);
  //Number of stored tokens
  uint8_t tokens_size_ = 0;
  //Storage for tokens
  char *tokens_[SCPI_MAX_TOKENS];
  //Number of registered commands
  uint8_t codes_size_ = 0;
  //Registered commands' hash storage
  scpi_hash_t valid_codes_[SCPI_MAX_COMMANDS];
  //Pointers to the functions to be called when a valid command is received
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS+1];
  // Hash of the base path used to calculate command hashes (0 for the root).
  scpi_hash_t tree_code_ = 0;
  // Number of keywords in the base path (0 for the root).
  uint8_t tree_length_ = 0;
  //Message buffer.
  char msg_buffer_[SCPI_BUFFER_LENGTH];
  // Number of bytes read into the message buffer.
  uint8_t message_length_ = 0;
  // Time at which the last byte was read.
  unsigned long time_checker_;

  #if SCPI_MAX_SPECIAL_COMMANDS
  //Max number of registered special commands.
  const uint8_t max_special_commands = SCPI_MAX_SPECIAL_COMMANDS;
  //Number of registered special commands
  uint8_t special_codes_size_ = 0;
  //Registered special commands' hash storage
  scpi_hash_t valid_special_codes_[SCPI_MAX_SPECIAL_COMMANDS];
  //Pointers to the functions to be called when a special command is received
  SCPI_special_caller_t special_callers_[SCPI_MAX_SPECIAL_COMMANDS];
  #endif
};

// Include the implementation code here
// This allows Arduino IDE users to configure options with #define directives 
//
// Include the implementation only once, in the main .ino file.
// Before including the header in other .cpp files, use the following directives.
//
// #define VREKRER_SCPI_PARSER_NO_IMPL
// #include "Vrekrer_scpi_parser.h"
//
// Including the implementation in multiple files causes a linker error.
//
// (.text+0x0): multiple definition of `SCPI_Parser::ProcessInput(Stream&, char const*)'

#ifndef VREKRER_SCPI_PARSER_NO_IMPL
#include "Vrekrer_scpi_arrays_code.h"
#include "Vrekrer_scpi_parser_code.h"
#include "Vrekrer_scpi_parser_special_code.h"
#endif

#endif //VREKRER_SCPI_PARSER_H_
