#ifndef SCPI_parser_h
#define SCPI_parser_h

// Maximun size of commnad tree and number of parameters
#ifndef SCPI_ARRAY_SYZE
#define SCPI_ARRAY_SYZE 6
#endif 

#ifndef SCPI_MAX_TOKENS
#define SCPI_MAX_TOKENS 15
#endif 

// Maximun number of registered commands 
#ifndef SCPI_MAX_COMMANDS
#define SCPI_MAX_COMMANDS 20
#endif 

#include "Arduino.h"

/** Class which contains methods for storing and retrieving individual SCPI commands and parameters, which
 * are handled as strings (character arrays). 
 */
class SCPI_String_Array {
 public:
  SCPI_String_Array();
  ~SCPI_String_Array();
  char* operator[](const byte index);
  void Append(char* value);
  char* Pop();
  char* First();
  char* Last();
  uint8_t Size();
 protected:
  /** Number of strings (?) or is it the number of characters */
  uint8_t size_ = 0;
  char* values_[SCPI_ARRAY_SYZE];
};

/** Class which handles SCPI commands. Takes an input SCPI message and parses it to extract the commands
 * What kind of command structure does this assume?
 */
class SCPI_Commands : public SCPI_String_Array {
 public:
  SCPI_Commands();
  SCPI_Commands(char* message);
  /** Pointer to unprocessed string */
  char* not_processed_message; 
};

/** Class which handles SCPI parameters. Takes an input message and parses it to extract the parameters.
 * What kind of command structure does this assume?
 */
class SCPI_Parameters : public SCPI_String_Array {
 public:
  SCPI_Parameters();
  SCPI_Parameters(char *message);
  /** Pointer to unprocessed string */
  char* not_processed_message;
};

// Unclear what these typedefs do 
typedef SCPI_Commands SCPI_C;
typedef SCPI_Parameters SCPI_P;
typedef void (*SCPI_caller_t)(SCPI_C, SCPI_P, Stream&);

/** Main class for parsing SCPI messages into commands and parameters. This looks like the big daddy class
 * that does all the heavy lifting. It looks like the SCPI Parameters and Commands classes each do their
 * own parsing, what is the unique purpose of the parser class? It seems more like a communication/
 * interface class than a 'Parser'. Might want to just rename to 'SCPI'
 */
class SCPI_Parser {
 public:
  void SetCommandTreeBase(const char* tree_base);
  void SetCommandTreeBase(const __FlashStringHelper* tree_base);
  void RegisterCommand(const char* command, SCPI_caller_t caller);
  void RegisterCommand(const __FlashStringHelper* command, SCPI_caller_t caller);
  void Execute(char* message, Stream& interface);
  void ProcessInput(Stream &interface, char* term_chars);
  char* GetMessage(Stream& interface, char* term_chars);
  void PrintDebugInfo();
 protected:
  void AddToken(char* token);
  uint32_t GetCommandCode(SCPI_Commands& commands);
  /** What does the tokens size represent? */
  uint8_t tokens_size_ = 0;
  /** What are the 'tokens' in this class? This is unclear */
  char *tokens_[SCPI_MAX_TOKENS];
  /** What does the codes size represent? */
  uint8_t codes_size_ = 0;
  /** What are the valid codes? */
  uint32_t valid_codes_[SCPI_MAX_COMMANDS];
  /** I have no idea what this is */
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS];
  /** What is a tree code? */
  uint32_t tree_code_ = 1;
  char msg_buffer[64]; //TODO BUFFER_LENGTH. Expand on TODO.
};

#endif 
