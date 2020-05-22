#include "Vrekrer_scpi_parser.h"
#include "Arduino.h"

// SCPI_String_Array member functions

SCPI_String_Array::SCPI_String_Array() {}

SCPI_String_Array::~SCPI_String_Array() {}

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

SCPI_Commands::SCPI_Commands() {}

SCPI_Commands::SCPI_Commands(char *message) {
  char* token = message;
  // Trim leading spaces
  while (isspace(*token)) token++;
  // Discard parameters and multicommands
  not_processed_message = strpbrk(token, " \t;");
  if (not_processed_message != NULL) {
    not_processed_message += 1;
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

SCPI_Parameters::SCPI_Parameters() {}

SCPI_Parameters::SCPI_Parameters(char* message) {
  char* parameter = message;
  // Discard parameters and multicommands
  not_processed_message = strpbrk(parameter, ";");
  if (not_processed_message != NULL) {
    not_processed_message += 1;
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
  //TODO add support for strings parameters
}


//SCPI_Registered_Commands member functions

void SCPI_Parser::AddToken(char *token) {
  //Strip '?' from end if needed
  size_t original_size = strlen(token);
  char *mytoken = strtok(token, "?");
  //add the token
  bool allready_added = false;
  for (uint8_t i = 0; i < tokens_size_; i++)
    allready_added ^= (strcmp(mytoken, tokens_[i]) == 0);
  if (!allready_added) {
    if (tokens_size_ < SCPI_MAX_TOKENS) {
      char *stored_token = new char [strlen(mytoken) + 1];
      strcpy(stored_token, mytoken);
      tokens_[tokens_size_] = stored_token;
      tokens_size_++;
    }
  }
  //Restore ? if needed (for SCPI_Parser::GetCommandCode processing)
  if (original_size > strlen(token)) token[original_size-1] = '?';
}

uint32_t SCPI_Parser::GetCommandCode(SCPI_Commands& commands) {
  uint32_t code = tree_code_ - 1;
  char* header;
  for (int i = 0; i < commands.Size(); i++) {
    code *= SCPI_MAX_TOKENS;
    header = commands[i];
    bool isToken;
    for (uint8_t j = 0; j < tokens_size_; j++) {
      size_t ss = 0; //Token Short size
      while (isupper(tokens_[j][ss])) ss++;
      size_t ls = strlen(tokens_[j]); //Token Long size
      size_t hs = strlen(header); //Header size

      isToken = true;
      if ((hs == ss) | (hs == ss+1)) { //short token
        for (uint8_t k  = 0; k < ss; k++)
          isToken &= (toupper(header[k]) == tokens_[j][k]);
      } else if ((hs == ls) | (hs == ls+1)) { //long token
        for (uint8_t k  = 0; k < ls; k++)
          isToken &= (toupper(header[k]) == toupper(tokens_[j][k]));
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
  if (header[strlen(header) - 1] == '?') code ^= 0x80000000;
  return code+1;
}

void SCPI_Parser::SetCommandTreeBase(const __FlashStringHelper* tree_base) {
  strcpy_P(msg_buffer, (const char *) tree_base);
  this->SetCommandTreeBase(msg_buffer);
}

void SCPI_Parser::SetCommandTreeBase(const char* tree_base) {
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
    if (valid_codes_[i] == code)
      (*callers_[i])(commands, parameters, interface);
}

char* SCPI_Parser::GetMessage(Stream& interface, char* term_chars) {
  uint8_t msg_counter = 0;
  msg_buffer[msg_counter] = '\0';

  bool continous_data = false;
  unsigned long last_data_millis = millis();
  do {
    if (interface.available()) {
        continous_data = true;
        last_data_millis = millis();
        msg_buffer[msg_counter] =  interface.read();

        //TODO check msg_counter overflow
        ++msg_counter;
        msg_buffer[msg_counter] = '\0';

        if (strstr(msg_buffer, term_chars) != NULL) {
          msg_buffer[msg_counter - strlen(term_chars)] =  '\0';
          break;
        }
    } else { //No chars aviable jet
      if ((millis() - last_data_millis) > 10) // 10 ms without new data
        continous_data = false;
    }
  } while (continous_data);
  if (continous_data)
    return msg_buffer;
  else
    return NULL;
}

void SCPI_Parser::ProcessInput(Stream& interface, char* term_chars) {
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
