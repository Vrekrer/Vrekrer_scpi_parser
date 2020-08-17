#ifndef SCPI_parser_h
#define SCPI_parser_h

/*
Usage:
See the examples
TODO make a wiki.
*/


/*
Definitions:

Termination chars.
 A string formed by any characters that indicates the end of a message.
 Usual termination chars are LineFeed '\n' and/or CarrierReturn '\r'.
 e.g.
  "\n"
  "\r\n"

Message.
 A string that could contain one or more commands and parameters, and is 
 terminated by termination chars.
 e.g.
  "*IDN?; SYSTEM:TIMER:SET 5, minutes; syst:led:brig:inc\n"
  "This message does not contain valid commands.\r\n"

Tokens.
 Words (strings) used to form a command.
 e.g.
  "SYSTEM", "syst", "*IDN", "TIMER", "SET"
 Tokens are not case sesitive.
 Tokens usualy have a long form (complete word) and a short form (partial word).
 To define valid tokens we use upper case for the short form and lower case to
 complete the long form (if needed).
 e.g.
  "SYSTem"
  Sort form : "SYST" 
  Long form : "system"

Commands.
 One or more tokens separeted by the ':' character.
 e.g.
  "*IDN?", "system:timer:set"
 A command terminated in the '?' character is defined as a "query".
 Inside a message commands are separated by the ';' character.
 
Parameters.
 Comma ',' separated strings that comes after a command in a message.
 e.g.
  For the message "SYSTEM:TIMER:SET 5, minutes" the parameters are:
   "5" and "minutes"
 Spaces between parameters are ignored.

Valid commands / Command tree.
 Set of valid commands, usualy defined in a branched form.
 The ':' token separator defines the branches
 e.g.
  "SYSTem:LED:BRIGhtness:INCrease" (Valid command with branch size 4)
  "SYSTem:LED:BRIGhtness:DEcrease"
  Both commands share the TreeBase "SYSTem:LED:BRIGhtness"
*/


// Maximum branch size of the command tree and maximum number of parameters.
#ifndef SCPI_ARRAY_SYZE
#define SCPI_ARRAY_SYZE 6
#endif 

// Maximum number of valid tokens.
#ifndef SCPI_MAX_TOKENS
#define SCPI_MAX_TOKENS 15
#endif 

// Maximum number of registered commands.
#ifndef SCPI_MAX_COMMANDS
#define SCPI_MAX_COMMANDS 20
#endif 

// Length of the message buffer.
#ifndef SCPI_BUFFER_LENGTH
#define SCPI_BUFFER_LENGTH 64
#endif 

#include "Arduino.h"

/*!
  @brief Variable size string array class.

  The array must be filled using the Append method (acts as a LIFO stack Push).
  Values can be extracted (and removed) using the Pop function (LIFO stack Pop).
  Both Append and Pop modifies the Size of the array.
  Values can be read (without removing them) using the following methods:
    First() : Returns the fist value appended to the array
    Last()  : Returns the last value appended to the array
    Indexing (e.g. my_array[1] to get the second value of the array)
  The maximum size of the array is defined by SCPI_ARRAY_SYZE (default 6)
*/
class SCPI_String_Array {
 public:
  char* operator[](const byte index);  //Add indexing capability
  void Append(char* value);            //Append new string (LIFO stack Push)
  char* Pop();                         //LIFO stack Pop
  char* First();                       //Returns the fist element of the array
  char* Last();                        //Returns the last element of the array
  uint8_t Size();                      //Current array size
 protected:
  uint8_t size_ = 0;              //Internal current array size 
  char* values_[SCPI_ARRAY_SYZE]; //Storage of the strings
};

/*!
  @brief String array class used to store the tokens of a command.
  
  This class inherits the properties of SCPI_String_Array.
  @see SCPI_String_Array
*/
class SCPI_Commands : public SCPI_String_Array {
 public:
  /*!
    @brief Constructor that extracts and tokenize a command from a message.
    @param message to process.
    
    The message is procesed until a space, ';' or the end of the string is 
    found, the rest is available at not_processed_message.
    The processed part is splitted on the ':' characters, the resulting parts 
    (tokens) are stored in the array.
  */
  SCPI_Commands(char* message);
  /*!
    @brief Not processed part of the message after the constructor is called.
  */
  char* not_processed_message;
};


/*!
  @brief String array class used to store the parameters found after a command.
  
  This class inherits the properties of SCPI_String_Array.
  @see SCPI_String_Array
*/
class SCPI_Parameters : public SCPI_String_Array {
 public:
  /*!
    @brief Constructor that extracts and splits parameters from a message.
    @param message to process.

    The message is procesed until ';' or the end of the string is found, 
    the rest is available at not_processed_message.
    The processed part is splitted on the ',' characters, the resulting parts
    (pamareters) are stored in the array after trimming any start or end spaces.
  */
  SCPI_Parameters(char *message);
  /*!
    @brief Not processed part of the message after the constructor is called.
  */
  char* not_processed_message;
};

typedef SCPI_Commands SCPI_C;   //Alias of SCPI_Commands
typedef SCPI_Parameters SCPI_P; //Alias of SCPI_Parameters

/*!
  @brief void template used with SCPI_Parser::RegisterCommand
*/
typedef void (*SCPI_caller_t)(SCPI_Commands, SCPI_Parameters, Stream&);

/*!
  @brief Main class of the Vrekrer_SCPI_Parser library
*/
class SCPI_Parser {
 public:
  /*!
    @brief Change the TreeBase for the next RegisterCommand calls.
    @param tree_base TreeBase to be used.
                     An empty string "" sets the TreeBase to root.
  */
  void SetCommandTreeBase(char* tree_base);

  /*!
    @brief SetCommandTreeBase version with RAM string support.
    e.g.  my_instrument.SetCommandTreeBase("SYSTem:LED");
    For lower RAM usage you shoud use the Flash strings version
  */
  void SetCommandTreeBase(const char* tree_base);

 /*!
    @brief SetCommandTreeBase version with Flash strings (F() macro) support.
    e.g. my_instrument.SetCommandTreeBase(F("SYSTem:LED"));
  */
  void SetCommandTreeBase(const __FlashStringHelper* tree_base);

  /*!
    @brief Registers a new valid command and associate a procedure to it.
    @param command new valid command.
    @param caller procedure associated to the valid command.
  */
  void RegisterCommand(char* command, SCPI_caller_t caller);

  /*!
    @brief RegisterCommand version with RAM string support.
    e.g. my_instrument.RegisterCommand("*IDN?", &Identify);
    For lower RAM usage you shoud use the Flash strings version
  */
  void RegisterCommand(const char* command, SCPI_caller_t caller);

  /*!
    @brief RegisterCommand version with Flash strings (F() macro) support.
    e.g. my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  */
  void RegisterCommand(const __FlashStringHelper* command, SCPI_caller_t caller);

  /*!
    @brief Process a message to obtain commands and parameters.
           If a valid command is found, its associated procedure is executed.
           The command' tokens and parameters, and the interface is passed
           to the executed procedure.
    @param message message.to be processed.
    @param interface the source of the message.
    @see GetMessage
  */
  void Execute(char* message, Stream& interface);

  /*!
    @brief Gets a message from an Stream interface and execute it
    @see GetMessage
    @see Execute
  */
  void ProcessInput(Stream &interface, const char* term_chars);

  /*!
    @brief Gets a message from an Stream interface.
           Reads the available chars in the interface until the term_chars are 
           found. If the term_chars are not found after 10 ms (hard coded for 
           now) all the readed chars are discarded.
           Internally this function uses a buffer with SCPI_BUFFER_LENGTH 
           length, no overflow check is done (yet).
    @param interface a Stream interface like Serial or Ethernet.
    @param term_chars termination chars e.g. "\r\n".
    @return the read message.
  */
  char* GetMessage(Stream& interface, const char* term_chars);

  /*!
    @brief Prints some debug info
    TODO Document this function.
  */
  void PrintDebugInfo();
 protected:
  //Add a token to the valid tokens' storage
  void AddToken(char* token);
  //Get a unique code from a command
  uint32_t GetCommandCode(SCPI_Commands& commands);
  //Number of current valid tokens
  uint8_t tokens_size_ = 0;
  //Storage of valid tokens
  char *tokens_[SCPI_MAX_TOKENS];
  //Number of current valid commands
  uint8_t codes_size_ = 0;
  //Storage of unique codes of the valid commands
  uint32_t valid_codes_[SCPI_MAX_COMMANDS];
  //Pointers to the functions to be called when a valid command is recieved
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS];
  //Branch offset used when calculating unique codes (1 for root)
  uint32_t tree_code_ = 1;
  //Message buffer.
  char msg_buffer[SCPI_BUFFER_LENGTH];
};

#endif
