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

#include "Arduino.h";

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
  uint8_t size_ = 0;
  char* values_[SCPI_ARRAY_SYZE];
};

class SCPI_Commands : public SCPI_String_Array {
 public:
  SCPI_Commands();
  SCPI_Commands(char* message);
  char* not_processed_message;
};

class SCPI_Parameters : public SCPI_String_Array {
 public:
  SCPI_Parameters();
  SCPI_Parameters(char *message);
  char* not_processed_message;
};

typedef SCPI_Commands SCPI_C;
typedef SCPI_Parameters SCPI_P;
typedef void (*SCPI_caller_t)(SCPI_C, SCPI_P, Stream&);

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
  void PrintCommands(Stream& interface);
 protected:
  void AddToken(char* token);
  uint32_t GetCommandCode(SCPI_Commands& commands);
  uint8_t tokens_size_ = 0;
  char *tokens_[SCPI_MAX_TOKENS];
  uint8_t codes_size_ = 0;
  uint32_t valid_codes_[SCPI_MAX_COMMANDS];
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS];
  uint32_t tree_code_ = 1;
  char msg_buffer[64]; //TODO BUFFER_LENGTH
};

#endif 
