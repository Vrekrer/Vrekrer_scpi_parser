#ifndef SCPI_parser_h
#define SCPI_parser_h

// Maximun size of commnad tree and number of parameters
#ifndef SCPI_ARRAY_SYZE
#define SCPI_ARRAY_SYZE 6
#endif 

#ifndef SCPI_MAX_TOKENS
#define SCPI_MAX_TOKENS 15
#endif 

// Maximun number of register commands 
#ifndef SCPI_MAX_COMMANDS
#define SCPI_MAX_COMMANDS 20
#endif 

#include "Arduino.h";

class SCPI_String_Array {
 public:
  String operator[](const byte index);
  void Append(String value);
  byte Size();
 protected:
  byte size_ = 0;
  String values_[SCPI_ARRAY_SYZE];
};

class SCPI_Commands : public SCPI_String_Array {
 public:
  SCPI_Commands();
  SCPI_Commands(String message);
};

class SCPI_Parameters : public SCPI_String_Array {
 public:
  SCPI_Parameters();
  SCPI_Parameters(String message);
};

typedef void (*SCPI_caller_t)(SCPI_Commands, SCPI_Parameters);

class SCPI_Parser {
 public:
  void RegisterCommand(String command, SCPI_caller_t caller);
  void Execute(String message);
 protected:
  void AddToken(String token);
  String GetCommandCode(SCPI_Commands commands);
  byte tokens_size_ = 0;
  String tokens_[SCPI_MAX_TOKENS];
  byte codes_size_ = 0;
  String valid_codes_[SCPI_MAX_COMMANDS];
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS];
  String scope_;
};

#endif
