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

byte SCPI_String_Array::Size() {
  return size_;
}

// SCPI_Commands member functions

SCPI_Commands::SCPI_Commands() {}

SCPI_Commands::SCPI_Commands(String message) {
  int i;
  // Discard parameters
  message.trim();
  i = message.indexOf(' ');
  if (i != -1)
    message.remove(i);
  // Strip using ':'
  while (message.length()) {
    i = message.indexOf(':');
    if (i == 0) {
      message.remove(0, 1);
    } else if (i != -1) {
      this->Append(message.substring(0, i));
      message.remove(0, i + 1);
    } else {
      this->Append(message);
      message.remove(0);
    }
  }
}

// SCPI_Parameters member functions

SCPI_Parameters::SCPI_Parameters() {}

SCPI_Parameters::SCPI_Parameters(String message) {
  int i;
  // Discard commands
  message.trim();
  i = message.indexOf(' ');
  if (i != -1) {
    message.remove(0, i + 1);
    message.trim();
  } else { // No parameters
    return;
  }
  // Strip using  parameter separator ','
  while (message.length()) {
    i = message.indexOf(',');
    //Check for string parameters enclosed by "" or ''
    if (message.charAt(0) == '"') {
      i = message.indexOf('"', 1);
      if (i != -1) ++i;
    }
    if (message.charAt(0) == '\'') {
      i = message.indexOf('\'', 1);
      if (i != -1) ++i;
    }

    if (i == 0) {
      this->Append(String("")); // Empty parameter 
      message.remove(0, 1);
    } else if (i != -1) {
      this->Append(message.substring(0, i));
      message.remove(0, i + 1);
    } else {
      this->Append(message);
      message.remove(0);
    }
    message.trim();
  }
}

//SCPI_Registered_Commands member functions

void SCPI_Parser::AddToken(String token) {
  //Strip '?' from end if needed
  if (token.endsWith("?"))
    token.remove(token.length() -1);

  // Read only the initial upper case chars 
  int i;
  for (i = 0; i < token.length(); i++)
    if (isLowerCase(token.charAt(i))) break;
  token.remove(i);

  //add the token
  bool allready_added = false;
  for (i = 0; i < tokens_size_; i++)
    allready_added ^= token.equals(tokens_[i]);
  if (!allready_added) {
    tokens_[tokens_size_] = token;
    tokens_size_++;
  }
}

String SCPI_Parser::GetCommandCode(SCPI_Commands commands) {
  char code[10]; //TODO Max Command nesting?
  int i;
  for (i = 0; i < commands.Size(); i++) {
    code[i] = char(33);  //Default value ch(33) =!
    String commm = commands[i];
    commm.toUpperCase();
    byte offset = (commm.endsWith("?")) ? 65 : 97;  //ch(65) = A; ch(97) = a
    for (int j = 0; j < tokens_size_; j++)
      if (commm.startsWith(tokens_[j]))
        code[i] = char(offset + j);
  }
  code[i] = '\0';
  return String(code);
}

void SCPI_Parser::RegisterCommand(String command, SCPI_caller_t caller) {
  SCPI_Commands commands(command);
  for (int i = 0; i < commands.Size(); i++)
    this->AddToken(commands[i]);
  String code = this->GetCommandCode(commands);
  valid_codes_[codes_size_] = code;
  callers_[codes_size_] = caller;
  codes_size_++;
}

void SCPI_Parser::Execute(String message) {
  SCPI_Commands commands(message);
  SCPI_Parameters parameters(message);
  String code = this->GetCommandCode(commands);
  for (byte i = 0; i < codes_size_; i++)
    if (valid_codes_[i].equals(code) && callers_[i]) {
      (*callers_[i])(commands, parameters);
      return;
    }
}
