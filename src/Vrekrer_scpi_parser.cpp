#include "Vrekrer_scpi_parser.h"

// SCPI_String_Array member functions

char* SCPI_String_Array::operator[](const uint8_t index) {
  return values_[index];
}

void SCPI_String_Array::Append(char* value) {
  if (size_ < SCPI_ARRAY_SYZE) {
    values_[size_] = value;
    size_++;
  }
}

char* SCPI_String_Array::Pop() {
  if (size_ > 0) {
    size_--;
    return values_[size_];
  } else {
    return NULL;
  }
}

char* SCPI_String_Array::First() {
  if (size_ > 0) {
    return values_[0];
  } else {
    return NULL;
  }
}

char* SCPI_String_Array::Last() {
  if (size_ > 0) {
    return values_[size_ - 1];
  } else {
    return NULL;
  }
}

uint8_t SCPI_String_Array::Size() {
  return size_;
}

// SCPI_Commands member functions
SCPI_Commands::SCPI_Commands(){}

SCPI_Commands::SCPI_Commands(char *message) {
  char* token = message;
  // Trim leading spaces
  while (isspace(*token)) token++;
  // Discard parameters and multicommands
  this->not_processed_message = strpbrk(token, " \t;");
  if (this->not_processed_message != NULL) {
    this->not_processed_message += 1;
    token = strtok(token, " \t;");
    token = strtok(token, ":");
  } else {
    token = strtok(token, ":");
  }
  // Strip using ':'
  while (token != NULL) {
    this->Append(token);
    token = strtok(NULL, ":");
  }
}

// SCPI_Parameters member functions
SCPI_Parameters::SCPI_Parameters(){}

SCPI_Parameters::SCPI_Parameters(char* message) {
  char* parameter = message;
  // Discard parameters and multicommands
  this->not_processed_message = strpbrk(parameter, ";");
  if (this->not_processed_message != NULL) {
    this->not_processed_message += 1;
    parameter = strtok(parameter, ";");
    parameter = strtok(parameter, ",");
  } else {
    parameter = strtok(parameter, ",");
  }
  // Strip using ':'
  while (parameter != NULL) {
    while(isspace(*parameter)) parameter++;
    this->Append(parameter);
    parameter = strtok(NULL, ",");
  }
  //TODO add support for strings parameters (do not split parameters inside "")
}

//Do nothing function
void DefaultErrorHandler(SCPI_C c, SCPI_P p, Stream& interface) {}

//SCPI_Registered_Commands member functions
SCPI_Parser::SCPI_Parser(){
  callers_[SCPI_MAX_COMMANDS] = &DefaultErrorHandler;
}

void SCPI_Parser::SetErrorHandler(SCPI_caller_t caller){
  callers_[SCPI_MAX_COMMANDS] = caller;
}

void SCPI_Parser::AddToken(char *token) {
  size_t token_size = strlen(token);
  bool isQuery = (token[token_size - 1] == '?');
  if (isQuery) token_size--;

  bool allready_added = false;
  for (uint8_t i = 0; i < tokens_size_; i++)
    allready_added ^= (strncmp(token, tokens_[i], token_size) == 0);
  if (!allready_added) {
    if (tokens_size_ < SCPI_MAX_TOKENS) {
      char *stored_token = new char [token_size + 1];
      strncpy(stored_token, token, token_size);
      stored_token[token_size] = '\0';
      tokens_[tokens_size_] = stored_token;
      tokens_size_++;
    }
  }
}

uint32_t SCPI_Parser::GetCommandCode(SCPI_Commands& commands) {
  //TODO Use a hash function instead of "base_SCPI_MAX_TOKENS numbers".
  uint32_t code = tree_code_ - 1; // tree_code = 1 when execute
  bool isQuery = false;
  for (uint8_t i = 0; i < commands.Size(); i++) {
    code *= SCPI_MAX_TOKENS;
    size_t header_length = strlen(commands[i]);  //header's length
    if (i == commands.Size() - 1) { //Last header
      isQuery = (commands[i][header_length - 1] == '?');
      if (isQuery) header_length--;
    }
    
    bool isToken;
    for (uint8_t j = 0; j < tokens_size_; j++) {
      size_t short_length = 0; //short token's length
      while (isupper(tokens_[j][short_length])) short_length++;
      size_t long_length = strlen(tokens_[j]); //long token's length

      if ( (tokens_[j][long_length - 1] == '#') //Numeric suffix capable token
         && (commands[i][header_length - 1] != '#') ) {
        long_length--;
        while (isdigit(commands[i][header_length - 1])) header_length--;
      }

      isToken = true;
      if (header_length == short_length) { //match with short token
        for (uint8_t k  = 0; k < short_length; k++)
          isToken &= (toupper(commands[i][k]) == tokens_[j][k]);
      } else if (header_length == long_length) { //match with long token
        for (uint8_t k  = 0; k < long_length; k++)
          isToken &= (toupper(commands[i][k]) == toupper(tokens_[j][k]));
      } else {
        isToken = false;
      }
      if (isToken) {
        code += j;
        break;
      }
    }
    if (!isToken) return 0;
  }
  if (isQuery) code ^= 0x80000000;
  return code+1;
}

void SCPI_Parser::SetCommandTreeBase(const __FlashStringHelper* tree_base) {
  strcpy_P(msg_buffer, (const char *) tree_base);
  this->SetCommandTreeBase(msg_buffer);
}

void SCPI_Parser::SetCommandTreeBase(const char* tree_base) {
  strcpy(msg_buffer, tree_base);
  this->SetCommandTreeBase(msg_buffer);
}

void SCPI_Parser::SetCommandTreeBase(char* tree_base) {
  if (strlen(tree_base) > 0) {
    SCPI_Commands tree_tokens(tree_base);
    for (uint8_t i = 0; i < tree_tokens.Size(); i++)
      this->AddToken(tree_tokens[i]);
    tree_code_ = 1;
    tree_code_ = this->GetCommandCode(tree_tokens);
  } else {
    tree_code_ = 1;
  }
}

void SCPI_Parser::RegisterCommand(const __FlashStringHelper* command, SCPI_caller_t caller) {
  strcpy_P(msg_buffer, (const char *) command);
  this->RegisterCommand(msg_buffer, caller);
}

void SCPI_Parser::RegisterCommand(const char* command, SCPI_caller_t caller) {
  strcpy(msg_buffer, command);
  this->RegisterCommand(msg_buffer, caller);
}

void SCPI_Parser::RegisterCommand(char* command, SCPI_caller_t caller) {
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken(command_tokens[i]);
  uint32_t code = this->GetCommandCode(command_tokens);
  valid_codes_[codes_size_] = code;
  callers_[codes_size_] = caller;
  codes_size_++;
}

void SCPI_Parser::Execute(char* message, Stream &interface) {
  tree_code_ = 1;
  SCPI_Commands commands(message);
  SCPI_Parameters parameters(commands.not_processed_message);
  uint32_t code = this->GetCommandCode(commands);
  for (uint8_t i = 0; i < codes_size_; i++)
    if (valid_codes_[i] == code) {
      (*callers_[i])(commands, parameters, interface);
      return;
    }
  //Call ErrorHandler UnknownCommand
  last_error = ErrorCode::UnknownCommand;
  (*callers_[SCPI_MAX_COMMANDS])(commands, parameters, interface);
}

char* SCPI_Parser::GetMessage(Stream& interface, const char* term_chars) {
  while (interface.available()) {
    //Read the new char
    msg_buffer[message_length] = interface.read();
    ++message_length;
    time_checker = millis();

    //Check for buffer overflow
    if (message_length >= SCPI_BUFFER_LENGTH){
      //Call ErrorHandler due BufferOverflow
      last_error = ErrorCode::BufferOverflow;
      (*callers_[SCPI_MAX_COMMANDS])(SCPI_C(), SCPI_P(), interface);
      message_length = 0;
      return NULL;
    }

    //Check for termination chars
    msg_buffer[message_length] = '\0';
    if (strstr(msg_buffer, term_chars) != NULL) {
      //Return the received message
      msg_buffer[message_length - strlen(term_chars)] =  '\0';
      message_length = 0;
      return msg_buffer;
    }
  }
  //No more chars aviable yet
  if (message_length == 0) return NULL;

  //Check for communication timeout
  if ((millis() - time_checker) > SCPI_TIMEOUT) {
      //Call ErrorHandler due Timeout
      last_error = ErrorCode::Timeout;
      (*callers_[SCPI_MAX_COMMANDS])(SCPI_C(), SCPI_P(), interface);
      message_length = 0;
      return NULL;
  }
}

void SCPI_Parser::ProcessInput(Stream& interface, const char* term_chars) {
  char* message = this->GetMessage(interface, term_chars);
  if (message != NULL) {
    this->Execute(message, interface);
  }
}

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


