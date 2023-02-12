/*! 
@file Vrekrer_scpi_parser.h 
Header file.
*/

#ifndef VREKRER_SCPI_PARSER_H_
#define VREKRER_SCPI_PARSER_H_

#define VREKRER_SCPI_VERSION "v0.4.3"


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

/// Integer size used for hashes.
#ifndef SCPI_HASH_TYPE
  #define SCPI_HASH_TYPE uint8_t
#endif

#include "Arduino.h"

/*!
 Variable size string array class.

 The array must be filled using the \c Append method (acts as a LIFO stack Push). \n
 Values can be extracted (and removed) using the \c Pop function (LIFO stack Pop). \n
 Both \c Append and \c Pop modifies the \c Size of the array. \n
 Values can be read (without removing them) using the following methods: \n
  \li \c First() : Returns the first value appended to the array.
  \li \c Last()  : Returns the last value appended to the array.
  \li Indexing (e.g. \c my_array[1] to get the second value of the array).

 The max size of the array is defined by \c SCPI_ARRAY_SYZE (default 6).
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
  const uint8_t storage_size = SCPI_ARRAY_SYZE; //Max size of the array 
  uint8_t size_ = 0;              //Internal array size
  char* values_[SCPI_ARRAY_SYZE]; //Storage of the strings
};

/*!
 String array class used to store the tokens of a command.
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

///Alias of SCPI_Commands.
typedef SCPI_Commands SCPI_C;

///Alias of SCPI_Parameters.
typedef SCPI_Parameters SCPI_P;

///Void template used with SCPI_Parser::RegisterCommand.
typedef void (*SCPI_caller_t)(SCPI_Commands, SCPI_Parameters, Stream&);

/// Integer size used for hashes.
typedef SCPI_HASH_TYPE scpi_hash_t;

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
  //Process a message and execute it a valid command is found
  void Execute(char* message, Stream& interface);
  //Gets a message from a Stream interface and execute it
  void ProcessInput(Stream &interface, const char* term_chars);
  //Gets a message from a Stream interface
  char* GetMessage(Stream& interface, const char* term_chars);
  //Prints registered tokens and command hashes to the serial interface
  void PrintDebugInfo();
  ///Magic number used for hashing the commands
  scpi_hash_t hash_magic_number = 37;
  
 protected:
  //Length of the message buffer.
  const uint8_t buffer_length = SCPI_BUFFER_LENGTH;
  //Max number of valid tokens.
  const uint8_t max_tokens = SCPI_MAX_TOKENS;
  //Max number of registered commands.
  const uint8_t max_commands = SCPI_MAX_COMMANDS;

  //Add a token to the tokens' storage
  void AddToken_(char* token);
  //Get a hash from a command
  scpi_hash_t GetCommandCode_(SCPI_Commands& commands);
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
  //Branch's hash used when calculating unique codes (0 for root)
  scpi_hash_t tree_code_ = 0;
  //Message buffer.
  char msg_buffer_[SCPI_BUFFER_LENGTH];
  //Length of the readed message
  uint8_t message_length_ = 0;
  //Timeout, in miliseconds, for GetMessage and ProcessInput.
  unsigned long timeout = 10;
  //Varible used for checking timeout errors
  unsigned long time_checker_;
};


// ## SCPI_String_Array member functions ##

///Add indexing capability.
char* SCPI_String_Array::operator[](const uint8_t index) {
  return values_[index];
}

///Append new string (LIFO stack Push).
void SCPI_String_Array::Append(char* value) {
  if (size_ < storage_size) {
    values_[size_] = value;
    size_++;
  }
}

///LIFO stack Pop
char* SCPI_String_Array::Pop() {
  if (size_ > 0) {
    size_--;
    return values_[size_];
  } else {
    return NULL;
  }
}

///Returns the first element of the array
char* SCPI_String_Array::First() {
  if (size_ > 0) {
    return values_[0];
  } else {
    return NULL;
  }
}

///Returns the last element of the array
char* SCPI_String_Array::Last() {
  if (size_ > 0) {
    return values_[size_ - 1];
  } else {
    return NULL;
  }
}

///Array size
uint8_t SCPI_String_Array::Size() {
  return size_;
}


// ## SCPI_Commands member functions ##

///Dummy constructor.
SCPI_Commands::SCPI_Commands(){}

/*!
 Constructor that extracts and tokenize a command from a message.  
 @param message  Message to process.

 The message is processed until a space, tab or the end of the string is 
 found, the rest is available at not_processed_message.  
 The processed part is split on the ':' characters, the resulting parts 
 (tokens) are stored in the array.
*/
SCPI_Commands::SCPI_Commands(char* message) {
  char* token = message;
  // Trim leading spaces and tabs
  while (isspace(*token)) token++;
  // Save parameters and multicommands for later
  not_processed_message = strpbrk(token, " \t");
  if (not_processed_message != NULL) {
   not_processed_message[0] = '\0';
   not_processed_message++;
  }
  // Split using ':'
  token = strtok(token, ":");
  while (token != NULL) {
    this->Append(token);
    token = strtok(NULL, ":");
  }
}


// ## SCPI_Parameters member functions ##

/// Dummy constructor.
SCPI_Parameters::SCPI_Parameters(){}


/*!
 Constructor that extracts and splits parameters from a message.  
 @param message[in,out]  Message to process.

 The message is split on the ',' characters, the resulting parts 
 (parameters) are stored in the array after trimming any start spaces.
*/
SCPI_Parameters::SCPI_Parameters(char* message) {
  char* parameter = message;
  // Split using ','
  parameter = strtok(parameter, ",");
  while (parameter != NULL) {
    while(isspace(*parameter)) parameter++;
    this->Append(parameter);
    parameter = strtok(NULL, ",");
  }
  //TODO add support for strings parameters (do not split parameters inside "")
}

//Do nothing function
void DefaultErrorHandler(SCPI_C c, SCPI_P p, Stream& interface) {}


// ## SCPI_Registered_Commands member functions. ##

/*!
 SCPI_Parser constructor.

 Example:  
  ``SCPI_Parser my_instrument``;
*/
SCPI_Parser::SCPI_Parser(){
  callers_[max_commands] = &DefaultErrorHandler;
}

///Add a token to the tokens' storage
void SCPI_Parser::AddToken_(char *token) {
  size_t token_size = strlen(token);
  bool isQuery = (token[token_size - 1] == '?');
  if (isQuery) token_size--;

  bool allready_added = false;
  for (uint8_t i = 0; i < tokens_size_; i++)
    allready_added ^= (strncmp(token, tokens_[i], token_size) == 0);
  if (!allready_added) {
    if (tokens_size_ < max_tokens) {
      char *stored_token = new char [token_size + 1];
      strncpy(stored_token, token, token_size);
      stored_token[token_size] = '\0';
      tokens_[tokens_size_] = stored_token;
      tokens_size_++;
    }
  }
}

/*!
 Get a hash from a valid command
 @param commands  Keywords of a command
 @return hash

 Return 0 if the command contains keywords not registered as tokens.  
 The hash is calculated including the TreeBase hash.  
 @see SetCommandTreeBase
*/
scpi_hash_t SCPI_Parser::GetCommandCode_(SCPI_Commands& commands) {
  scpi_hash_t code;
  if (tree_code_) {
    code = tree_code_;
  } else {
    code = 7; //Magic hash offset
  }
  bool isQuery = false;

  //Loop all keywords in the command
  for (uint8_t i = 0; i < commands.Size(); i++) {
    //Get keywords's length
    size_t header_length = strlen(commands[i]);
    
    //For the last keywords test if it is a query
    //and remove '?' from the keywords's length if needed.
    if (i == commands.Size() - 1) {
      isQuery = (commands[i][header_length - 1] == '?');
      if (isQuery) header_length--;
    }

    bool isToken;
    for (uint8_t j = 0; j < tokens_size_; j++) {
      //Get the token's short and long lengths
      size_t short_length = 0;
      while (isupper(tokens_[j][short_length])) short_length++;
      size_t long_length = strlen(tokens_[j]);

      //If the token allows numeric suffixes
      //remove the trailing digits from the header
      if ( (tokens_[j][long_length - 1] == '#')
         && (commands[i][header_length - 1] != '#') ) {
        long_length--;
        while (isdigit(commands[i][header_length - 1])) header_length--;
      }

      //Test if the header match with the token
      isToken = true;
      if (header_length == short_length) {
        for (uint8_t k  = 0; k < short_length; k++)
          isToken &= (toupper(commands[i][k]) == tokens_[j][k]);
      } else if (header_length == long_length) {
        for (uint8_t k  = 0; k < long_length; k++)
          isToken &= (toupper(commands[i][k]) == toupper(tokens_[j][k]));
      } else {
        isToken = false;
      }

      //We use the token number j for hashing
      //hash(i) = hash(i - 1) * hash_magic_number + j
      if (isToken) {
        code *= hash_magic_number;
        code += j;
        break;
      }
    }
    if (!isToken) return 255;
  }
  if (isQuery) {
    code *= 37;
    code -= 1;
  }
  return code;
}

/*!
 Change the TreeBase for the next RegisterCommand calls.
 @param tree_base  TreeBase to be used.  
        An empty string ``""`` sets the TreeBase to root.
*/
void SCPI_Parser::SetCommandTreeBase(char* tree_base) {
  if (strlen(tree_base) > 0) {
    SCPI_Commands tree_tokens(tree_base);
    for (uint8_t i = 0; i < tree_tokens.Size(); i++)
      AddToken_(tree_tokens[i]);
    tree_code_ = 0;
    tree_code_ = this->GetCommandCode_(tree_tokens);
  } else {
    tree_code_ = 0;
  }
}

/*!
 SetCommandTreeBase version with RAM string support.

 Example:  
 ``my_instrument.SetCommandTreeBase("SYSTem:LED");``  
 For lower RAM usage use the Flash strings version.
*/
void SCPI_Parser::SetCommandTreeBase(const char* tree_base) {
  strcpy(msg_buffer_, tree_base);
  this->SetCommandTreeBase(msg_buffer_);
}

/*!
 SetCommandTreeBase version with Flash strings (F() macro) support.

 Example:  
  ``my_instrument.SetCommandTreeBase(F("SYSTem:LED"));``
*/
void SCPI_Parser::SetCommandTreeBase(const __FlashStringHelper* tree_base) {
  strcpy_P(msg_buffer_, (const char *) tree_base);
  this->SetCommandTreeBase(msg_buffer_);
}

/*!
 Registers a new valid command and associate a procedure to it.
 @param command  New valid command.
 @param caller  Procedure associated to the valid command.
*/
void SCPI_Parser::RegisterCommand(char* command, SCPI_caller_t caller) {
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken_(command_tokens[i]);
  scpi_hash_t code = this->GetCommandCode_(command_tokens);
  valid_codes_[codes_size_] = code;
  callers_[codes_size_] = caller;
  codes_size_++;
}

/*!
 RegisterCommand version with RAM string support.

 Example:  
  ``my_instrument.RegisterCommand("*IDN?", &Identify);``  
 For lower RAM usage use the Flash strings version.
*/
void SCPI_Parser::RegisterCommand(const char* command, SCPI_caller_t caller) {
  strcpy(msg_buffer_, command);
  this->RegisterCommand(msg_buffer_, caller);
}

/*!
 RegisterCommand version with Flash strings (F() macro) support.

 Example:  
  ``my_instrument.RegisterCommand(F("*IDN?"), &Identify);``
*/
void SCPI_Parser::RegisterCommand(const __FlashStringHelper* command, SCPI_caller_t caller) {
  strcpy_P(msg_buffer_, (const char *) command);
  this->RegisterCommand(msg_buffer_, caller);
}

/*!
 Set the function to be used by the error handler.

 Example:  
  ``my_instrument.SetErrorHandler(&myErrorHandler);``
*/
void SCPI_Parser::SetErrorHandler(SCPI_caller_t caller){
  callers_[max_commands] = caller;
}


/*!
 Process a message and execute it if a valid command is found.
 @param message  Message to be processed.
 @param interface  The source of the message.
 
 Commands and parameters are extracted from the message,  
 if a valid command is found, its associated procedure is executed.  
 The command' tokens and parameters, and the interface is passed
 to the executed procedure.  
 @see GetMessage
*/
void SCPI_Parser::Execute(char* message, Stream &interface) {
  while (message != NULL) {
    //Save multicomands for later
    char* multicomands = strpbrk(message, ";");
    if (multicomands != NULL) {
     multicomands[0] = '\0';
     multicomands++;
    }

    tree_code_ = 0;
    SCPI_Commands commands(message);
    SCPI_Parameters parameters(commands.not_processed_message);
    scpi_hash_t code = this->GetCommandCode_(commands);
    uint8_t i;
    for (i = 0; i < codes_size_; i++)
      if (valid_codes_[i] == code) {
        (*callers_[i])(commands, parameters, interface);
        break;
      }
    if (i==codes_size_) {
      //code not found in valid_codes_
      //Call ErrorHandler UnknownCommand
      last_error = ErrorCode::UnknownCommand;
      (*callers_[max_commands])(commands, parameters, interface);
    }
    message = multicomands;
  }
}

/*!
 Gets a message from a Stream interface and execute it.
 @see GetMessage
 @see Execute
*/
void SCPI_Parser::ProcessInput(Stream& interface, const char* term_chars) {
  char* message = this->GetMessage(interface, term_chars);
  if (message != NULL) {
    this->Execute(message, interface);
  }
}

/*!
 Gets a message from a Stream interface.
 @param interface  A Stream interface like Serial or Ethernet.
 @param term_chars  Termination chars e.g. ``"\r\n"``.
 @return the read message if the ``term_chars`` are found, otherwise ``NULL``.

 Reads the available chars in the interface, if the term_chars are found
 the message is returned, otherwise the return is ``NULL``.  
 Subsequent calls to this function continues the message reading.  
 The message is discarded, and the error handler is called if:  
  A timeout occurs (10 ms without new chars)  
  The message buffer overflows
*/
char* SCPI_Parser::GetMessage(Stream& interface, const char* term_chars) {
  while (interface.available()) {
    //Read the new char
    msg_buffer_[message_length_] = interface.read();
    ++message_length_;
    time_checker_ = millis();

    if (message_length_ >= buffer_length){
      //Call ErrorHandler due BufferOverflow
      last_error = ErrorCode::BufferOverflow;
      (*callers_[max_commands])(SCPI_C(), SCPI_P(), interface);
      message_length_ = 0;
      return NULL;
    }

    //Test for termination chars (end of the message)
    msg_buffer_[message_length_] = '\0';
    if (strstr(msg_buffer_, term_chars) != NULL) {
      //Return the received message
      msg_buffer_[message_length_ - strlen(term_chars)] =  '\0';
      message_length_ = 0;
      return msg_buffer_;
    }
  }
  //No more chars aviable yet

  //Return NULL if no message is incomming
  if (message_length_ == 0) return NULL;

  //Check for communication timeout
  if ((millis() - time_checker_) > timeout) {
      //Call ErrorHandler due Timeout
      last_error = ErrorCode::Timeout;
      (*callers_[max_commands])(SCPI_C(), SCPI_P(), interface);
      message_length_ = 0;
      return NULL;
  }

  //No errors, be sure to return NULL
  return NULL;
}

///Prints registered tokens and command hashes to the serial interface
void SCPI_Parser::PrintDebugInfo() {
  Serial.println(F("*** DEBUG INFO ***"));
  Serial.println();
  Serial.print(F("TOKENS :"));
  Serial.println(tokens_size_);
  for (uint8_t i = 0; i < tokens_size_; i++) {
    Serial.print(F("  "));
    Serial.println(String(tokens_[i]));
    Serial.flush();
  }
  Serial.println();
  Serial.println(F("VALID CODES :"));
  for (uint8_t i = 0; i < codes_size_; i++) {
    Serial.print(F("  "));
    Serial.println(valid_codes_[i]);
    Serial.flush();
  }
  Serial.println();
  Serial.println(F("*******************"));
  Serial.println();
}

#endif //VREKRER_SCPI_PARSER_H_
