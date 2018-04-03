#include "Vrekrer_scpi_parser.h";
#include "Arduino.h";

// SCPI_String_Array member functions

String SCPI_String_Array::operator[](const byte index) {
  return values_[index];
}

void SCPI_String_Array::Append(String value) {
  values_[size_] = value;
  size_++;
}

String SCPI_String_Array::Pop() {
  if (size_ > 0) {
    size_--;
    return values_[size_];
  } else {
    return String("");
  }
}

String SCPI_String_Array::First() {
  if (size_ > 0) {
    return values_[size_];
  } else {
    return String("");
  }
}

String SCPI_String_Array::Last() {
  if (size_ > 0) {
    return values_[size_ -1];
  } else {
    return String("");
  }
}

byte SCPI_String_Array::Size() {
  return size_;
}

// SCPI_Commands member functions

SCPI_Commands::SCPI_Commands() {}

SCPI_Commands::SCPI_Commands(String message) {
  int i;
  // Discard parameters and multicommands
  message.trim();
  i = message.indexOf(' ');
  if (i != -1) message.remove(i);
  i = message.indexOf(';');
  if (i != -1) message.remove(i);
  // Strip using ':'
  while (true) {
    i = message.indexOf(':');
    if (i == 0) { //leading ":"
      message.remove(0, 1);
    } else if (i != -1) { 
      this->Append(message.substring(0, i));
      message.remove(0, i + 1);
    } else { //last command
      this->Append(message);
      break;
    }
  }
}

// SCPI_Parameters member functions

SCPI_Parameters::SCPI_Parameters() {}

SCPI_Parameters::SCPI_Parameters(String message) {
  message.trim();
  int i = message.indexOf(' ');
  int j = message.indexOf(';');

  //remove commands before ' ' or ';'
  if ((j != -1) && (i != -1) && (j < i)) { //';' before ' '
    message.remove(0, j);
  } else if (i != -1) {
    message.remove(0, i + 1);
  } else if (j != -1) {
    message.remove(0, j);
  } else {  // No parameters
    return;
  }

  // Strip using  parameter separator ','
  // if it is multicommand message put next commands in last parameter
  while (message.length()) {
    message.trim();
    //Check for string parameters enclosed by "" or ''
    if ((message.charAt(0) == '"') || (message.charAt(0) == '\'')) {
      i = message.indexOf(message.charAt(0), 1);
      if (i == -1) {  //Bad unterminated string
        this->Append(message.substring(0, 1));
        break;
      } else {  //String found
        this->Append(message.substring(0, i+1));
        message.remove(0, i+1);
      }
      message.trim();
      if (message.charAt(0) == ',')  //remove next ',' if any
        message.remove(0, 1);
      continue;
    }
    //Split on "," or ";"
    i = message.indexOf(',');
    j = message.indexOf(';');
    if ((j == 0) && (message.length() == 1)) {  //just terminator
      break;
    } else if (j == 0) {  // This is a new commmand
      this->Append(message);  //Pass it as last parameter
      break;
    } else if ((j != -1) && (i != -1) && (j < i)) {  //';' before ' '
      this->Append(message.substring(0, j));
      message.remove(0, j);
    } else if (i != -1) {  //Normal Parameter
      this->Append(message.substring(0, i));
      message.remove(0, i+1);
    } else if (j != -1) {  //Parameter before new command
      this->Append(message.substring(0, j));
      message.remove(0, j);
    } else {  // Last parameter
      this->Append(message);
      break;
    }
  }
}

//SCPI_Registered_Commands member functions

void SCPI_Parser::AddToken(String token) {
  //Strip '?' from end if needed
  if (token.endsWith("?"))
    token.remove(token.length() -1);
  //add the token
  bool allready_added = false;
  for (int i = 0; i < tokens_size_; i++)
    allready_added ^= token.equals(tokens_[i]);
  if (!allready_added) {
    tokens_[tokens_size_] = token;
    tokens_size_++;
  }
}

String SCPI_Parser::GetCommandCode(SCPI_Commands commands) {
  char code[10]; //TODO Max Command nesting?
  for (int i = 0; i < commands.Size(); i++) {
    code[i] = char(33);  //Default value ch(33) =!
    String header = commands[i];
    header.toUpperCase();
    byte offset = 65;  //ch(65) = A
    if (header.endsWith("?")) {
      byte offset = 97;  //ch(65) = a
      header.remove(header.length(), 1);
    }
    for (int j = 0; j < tokens_size_; j++) {
      String short_token = tokens_[j];
      for (int t = 0; i < short_token.length(); t++)
        if (isLowerCase(short_token.charAt(t))) {
          short_token.remove(t);
          break;
        }
      String long_token = tokens_[j];
      long_token.toUpperCase();
      if (header.equals(short_token) || header.equals(long_token))
        code[i] = char(offset + j);
    }
  }
  code[commands.Size()] = '\0';
  return String(code);
}

void SCPI_Parser::SetCommandTreeBase(String tree_base) {
  SCPI_Commands tree_tokens(tree_base);
  for (int i = 0; i < tree_tokens.Size(); i++)
    this->AddToken(tree_tokens[i]);
  tree_code_ = this->GetCommandCode(tree_tokens);
}

void SCPI_Parser::RegisterCommand(String command, SCPI_caller_t caller) {
  SCPI_Commands command_tokens(command);
  for (int i = 0; i < command_tokens.Size(); i++)
    this->AddToken(command_tokens[i]);
  String code = this->GetCommandCode(command_tokens);
  valid_codes_[codes_size_] = tree_code_ + code;
  callers_[codes_size_] = caller;
  codes_size_++;
}

String SCPI_Parser::Execute_(String message) {
  Serial.println(String("Execute : ") + message);
  SCPI_Commands commands(message);
  SCPI_Parameters parameters(message);
  
  String not_processed_message = "";
  if (parameters.Last().startsWith(";")) { //It is a command not a param
    not_processed_message = parameters.Pop();
    not_processed_message.remove(0, 1);
  }

  String code = this->GetCommandCode(commands);
  String full_code = execute_scope_ + code;
  int i;
  for (i = 0; i < codes_size_; i++)
    if (valid_codes_[i].equals(code)) {
      code.remove(code.length() - 1);
      execute_scope_ = code;
      break;
    } else if (valid_codes_[i].equals(full_code)) {
      full_code.remove(full_code.length() - 1);
      execute_scope_ = full_code;
      break;
    }
  if (callers_[i] && (i < codes_size_)) {
    (*callers_[i])(commands, parameters);
  } else {
    execute_scope_ = String("");
  }
  return not_processed_message;
}

void SCPI_Parser::Execute(String message) {
  execute_scope_ = String("");
  while (message.length()) {
    message = this->Execute_(message);
  }
}
