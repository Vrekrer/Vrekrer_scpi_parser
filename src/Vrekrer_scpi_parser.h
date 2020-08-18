#ifndef VREKRER_SCPI_PARSER_H_
#define VREKRER_SCPI_PARSER_H_


// Maximum size of commnad tree and number of parameters.
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

// Timeout, in miliseconds, for GetMessage and ProcessInput.
#ifndef SCPI_TIMEOUT
  #define SCPI_TIMEOUT 10
#endif

#include "Arduino.h"

class SCPI_String_Array {
 public:
  char* operator[](const byte index);
  void Append(char* value);
  char* Pop();
  char* First();
  char* Last();
  uint8_t Size();
 protected:
  uint8_t size_ = 0;
  char* values_[SCPI_ARRAY_SYZE];
};

class SCPI_Commands : public SCPI_String_Array {
 public:
  SCPI_Commands(char* message);
  char* not_processed_message;
};

class SCPI_Parameters : public SCPI_String_Array {
 public:
  SCPI_Parameters(char *message);
  char* not_processed_message;
};

typedef SCPI_Commands SCPI_C;
typedef SCPI_Parameters SCPI_P;
typedef void (*SCPI_caller_t)(SCPI_Commands, SCPI_Parameters, Stream&);

class SCPI_Parser {
 public:
  void SetCommandTreeBase(char* tree_base);
  void SetCommandTreeBase(const char* tree_base);
  void SetCommandTreeBase(const __FlashStringHelper* tree_base);
  void RegisterCommand(char* command, SCPI_caller_t caller);
  void RegisterCommand(const char* command, SCPI_caller_t caller);
  void RegisterCommand(const __FlashStringHelper* command, SCPI_caller_t caller);
  void Execute(char* message, Stream& interface);
  void ProcessInput(Stream &interface, const char* term_chars);
  char* GetMessage(Stream& interface, const char* term_chars);
  void PrintDebugInfo();
  bool buffer_overflow = false;
  bool timeout = false;
 protected:
  void AddToken(char* token);
  uint32_t GetCommandCode(SCPI_Commands& commands);
  uint8_t tokens_size_ = 0;
  char *tokens_[SCPI_MAX_TOKENS];
  uint8_t codes_size_ = 0;
  uint32_t valid_codes_[SCPI_MAX_COMMANDS];
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS];
  uint32_t tree_code_ = 1;
  char msg_buffer[SCPI_BUFFER_LENGTH];
};

#endif //VREKRER_SCPI_PARSER_H_
