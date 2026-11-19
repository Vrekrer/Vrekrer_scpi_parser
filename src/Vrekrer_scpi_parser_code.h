// This file is included in Vrekrer_scpi_parser.h 
// This allows Arduino IDE users to configure options with #define directives 
// Do not include Vrekrer_scpi_parser.h here.


// Default callback that performs no action.
void DoNothing(SCPI_C, SCPI_P, Stream&) {}
static constexpr auto& DefaultErrorHandler = DoNothing;


// ## SCPI_Parser member functions ##

/*!
 SCPI_Parser constructor.

 Example:  
  ``SCPI_Parser my_instrument``;
*/
SCPI_Parser::SCPI_Parser(){
  callers_[max_commands] = &DefaultErrorHandler;
}

/// Adds a token to the token table.
void SCPI_Parser::AddToken_(char *token) {
  if (tokens_size_ >= max_tokens) {
    setup_errors.token_overflow = true;
    return;
  }
  size_t token_size = strlen(token);
  // Remove the query marker.
  if (token[token_size - 1] == '?') token_size--;
  for (uint8_t i = 0; i < tokens_size_; i++)
    // Check whether the token is already stored.
    if ( (strncmp(token, tokens_[i], token_size) == 0) 
          and (token_size == strlen(tokens_[i])) ) return;
  char *stored_token = new char [token_size + 1];
  strncpy(stored_token, token, token_size);
  stored_token[token_size] = '\0';
  tokens_[tokens_size_] = stored_token;
  tokens_size_++;
}

/*!
 Calculates the hash of a command.
 @param commands  Keywords of the command.
 @return The command hash, unknown_hash for an empty or unrecognized command,
         or invalid_hash if the base path is invalid.

 The calculation starts with the hash of the base path when one is set.
 @see SetCommandTreeBase
*/
scpi_hash_t SCPI_Parser::GetCommandCode_(SCPI_Commands& commands) {
  if (tree_code_ == invalid_hash) return invalid_hash;
  scpi_hash_t code;
  code = (tree_code_ == 0) ? hash_magic_offset : tree_code_;
  if (commands.Size()==0) return unknown_hash;
  //Loop all keywords in the command
  for (uint8_t i = 0; i < commands.Size(); i++) {
    // Get the length of the current keyword.
    size_t header_length = strlen(commands[i]);
    // Exclude the query marker from the last keyword when present.
    bool is_query = false;
    if (i == commands.Size() - 1) {
      is_query = (commands[i][header_length - 1] == '?');
      if (is_query) header_length--;
    }

    //Loop over all the known tokens
    for (uint8_t j = 0; j < tokens_size_; j++) {
      // Find the lengths of the token's short and long forms.
      size_t short_length = 0;
      while (isupper(tokens_[j][short_length])) short_length++;
      size_t long_length = strlen(tokens_[j]);

      // Ignore trailing digits when the token allows a numeric suffix.
      if ( (tokens_[j][long_length - 1] == '#')
         and (commands[i][header_length - 1] != '#') ) {
        long_length--;
        while (isdigit(commands[i][header_length - 1])) header_length--;
      }

      // Compare the keyword with the short or long form of the token.
      if (header_length == short_length) {
        for (uint8_t k  = 0; k < short_length; k++)
          if (not (toupper(commands[i][k]) == tokens_[j][k]))
            goto no_header_token_match;
      } else if (header_length == long_length) {
        for (uint8_t k  = 0; k < long_length; k++)
          if (not (toupper(commands[i][k]) == toupper(tokens_[j][k])))
            goto no_header_token_match;
      } else {
            goto no_header_token_match;
      }

      //Apply the hashing step using token number j
      //hash(i) = hash(i - 1) * hash_magic_number + j
      code *= hash_magic_number;
      code += j;
      break;

      no_header_token_match:;
      // Return unknown_hash after the last token fails to match.
      if (j == (tokens_size_ - 1)) return unknown_hash;
    }
    // Apply an extra hashing step for the query marker.
    if (is_query) {
      code *= hash_magic_number;
      code -= 1;
    }
  }
  return code;
}

/*!
 Sets the base path for subsequent command registrations.
 @param tree_base  The base path. An empty string selects the root.
*/
void SCPI_Parser::SetCommandTreeBase(char* tree_base) {
  SCPI_Commands tree_tokens(tree_base);
  if (tree_tokens.Size() == 0) {
    tree_code_ = 0;
    tree_length_ = 0;
    return;
  }
  for (uint8_t i = 0; i < tree_tokens.Size(); i++)
    AddToken_(tree_tokens[i]);
  tree_code_ = 0;
  tree_code_ = this->GetCommandCode_(tree_tokens);
  tree_length_ = tree_tokens.Size();
  if (tree_tokens.overflow_error) {
    setup_errors.branch_overflow = true;
    tree_code_ = invalid_hash;
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

SCPI_Parser::CommandCode SCPI_Parser::PrepareCommand_(char* command) {
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken_(command_tokens[i]);
  const scpi_hash_t code = this->GetCommandCode_(command_tokens);

  const int branch_length = tree_length_ + command_tokens.Size();
  const bool branch_too_long =
      command_tokens.overflow_error
      or (branch_length > command_tokens.storage_size);
  setup_errors.branch_overflow |= branch_too_long;

  const bool invalid =
      branch_too_long
      or (code == invalid_hash)
      or (code == unknown_hash);
  return {code, invalid};
}

/*!
 Registers a command and its callback.
 If registration fails and space remains, stores an invalid entry with DoNothing as its callback.
 @param command  New valid command.
 @param caller  Function to call when the command is received.
*/
void SCPI_Parser::RegisterCommand(char* command, SCPI_caller_t caller) {
  const CommandCode result = PrepareCommand_(command);

  if (codes_size_ >= max_commands) {
    setup_errors.command_overflow = true;
    return;
  }

  if (result.invalid) {
    valid_codes_[codes_size_] = invalid_hash;
    callers_[codes_size_] = DoNothing;
  } else {
    valid_codes_[codes_size_] = result.code;
    callers_[codes_size_] = caller;
  }
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
void SCPI_Parser::RegisterCommand(const __FlashStringHelper* command, 
                                  SCPI_caller_t caller) {
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
 Processes a message and executes any matching commands.
 @param message  Message to process.
 @param interface  Stream passed to each command callback.

 Each matching callback receives the command tokens, parameters, and stream.
 @see GetMessage
*/
void SCPI_Parser::Execute(char* message, Stream &interface) {
  while (message != NULL) {
    // Save the start of the next command in the message.
    char* next_command = strpbrk(message, ";");
    if (next_command != NULL) {
     next_command[0] = '\0';
     next_command++;
    }

    tree_code_ = 0;
    SCPI_Commands commands(message);
    message = next_command;
    SCPI_Parameters parameters(commands.not_processed_message);
    scpi_hash_t code = this->GetCommandCode_(commands);
    if (code == unknown_hash) {
      // Report an unknown command.
      last_error = ErrorCode::UnknownCommand;
      (*callers_[max_commands])(commands, parameters, interface);
      continue;
    }
    for (uint8_t i = 0; i < codes_size_; i++)
      if (valid_codes_[i] == code) {
        (*callers_[i])(commands, parameters, interface);
        break;
      }  
  }
}

/*!
 Reads a message from a Stream and executes any matching commands.
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
 Reads available input from a Stream.
 @param interface  Input stream, such as Serial or Ethernet.
 @param term_chars  Message terminator, such as ``"\r\n"``.
 @return A pointer to the message when one is available, or NULL.

 Subsequent calls resume reading a partial message. If the message buffer
 overflows or no new byte arrives within SCPI_Parser::timeout milliseconds
 (10 ms by default), the parser discards the message and calls the error handler.
*/
char* SCPI_Parser::GetMessage(Stream& interface, const char* term_chars) {
  while (interface.available()) {
    //Read the new char
    msg_buffer_[message_length_] = interface.read();
    ++message_length_;
    time_checker_ = millis();

    if (message_length_ >= buffer_length){
      // Report a message buffer overflow.
      last_error = ErrorCode::BufferOverflow;
      (*callers_[max_commands])(SCPI_C(), SCPI_P(), interface);
      message_length_ = 0;
      return NULL;
    }
    
    #if SCPI_MAX_SPECIAL_COMMANDS
    // Check for a special command at the first space.
    if (strcspn(msg_buffer_, " ") == message_length_- 1){
      msg_buffer_[message_length_ - 1] =  '\0';
      tree_code_ = 0;
      SCPI_Commands commands(msg_buffer_);
      scpi_hash_t code = this->GetCommandCode_(commands);
      for (uint8_t i = 0; i < special_codes_size_; i++) 
        if (valid_special_codes_[i] == code) {
          (*special_callers_[i])(commands, interface);
          message_length_ = 0;
          return msg_buffer_;
        }
      // Restore the original message for normal command parsing.
      msg_buffer_[message_length_ - 1] = ' ';
      for (uint8_t i = 0; i < commands.Size()-1; i++)
        commands[i][strlen(commands[i])] = ':';
      commands.not_processed_message--;
      commands.not_processed_message[0] = ' ';
    }
    #endif

    // Check for the message terminator.
    msg_buffer_[message_length_] = '\0';
    if (strstr(msg_buffer_, term_chars) != NULL) {
      //Return the received message
      msg_buffer_[message_length_ - strlen(term_chars)] =  '\0';
      message_length_ = 0;
      return msg_buffer_;
    }
  }
  // The stream has no more bytes available.

  // Return NULL if there is no partial message.
  if (message_length_ == 0) return NULL;

  //Check for communication timeout
  if ((millis() - time_checker_) > timeout) {
      // Report a timeout.
      last_error = ErrorCode::Timeout;
      (*callers_[max_commands])(SCPI_C(), SCPI_P(), interface);
      message_length_ = 0;
      return NULL;
  }

  // Wait for more input on the next call.
  return NULL;
}

///Prints debug information to an interface.
void SCPI_Parser::PrintDebugInfo(Stream& interface) 
{
  interface.println(F("*** DEBUG INFO ***\n"));
  interface.print(F("Max command tree branches: "));
  interface.print(SCPI_ARRAY_SIZE);
  interface.println(F(" (SCPI_ARRAY_SIZE)"));
  if (setup_errors.branch_overflow) 
    interface.println(F(" **ERROR** Max branch size exceeded."));
  interface.print(F("Max number of parameters: "));
  interface.print(SCPI_ARRAY_SIZE);
  interface.println(F(" (SCPI_ARRAY_SIZE)"));
  interface.print(F("Message buffer size: "));
  interface.print(buffer_length);
  interface.println(F(" (SCPI_BUFFER_LENGTH)\n"));
  
  interface.print(F("TOKENS : "));
  interface.print(tokens_size_);
  interface.print(F(" / "));
  interface.print(max_tokens);
  interface.println(F(" (SCPI_MAX_TOKENS)"));
  if (setup_errors.token_overflow) 
    interface.println(F(" **ERROR** Max tokens exceeded."));
  for (uint8_t i = 0; i < tokens_size_; i++) {
    interface.print(F("  "));
    interface.print(i+1);
    interface.print(F(":\t"));
    interface.println(String(tokens_[i]));
    interface.flush();
  }
  interface.println();
  
  bool hash_crash = false;
  bool unknown_error = false;
  bool invalid_error = false;
  interface.print(F("VALID CODES : "));
  interface.print(codes_size_);
  interface.print(F(" / "));
  interface.print(max_commands);
  interface.println(F(" (SCPI_MAX_COMMANDS)"));
  if (setup_errors.command_overflow) 
    interface.println(F(" **ERROR** Max commands exceeded."));
  interface.println(F("  #\tHash\t\tHandler"));
  for (uint8_t i = 0; i < codes_size_; i++) {
    interface.print(F("  "));
    interface.print(i+1);
    interface.print(F(":\t"));
    interface.print(valid_codes_[i], HEX);
    if (valid_codes_[i] == unknown_hash) {
      interface.print(F("!*"));
      unknown_error = true;
    } else if (valid_codes_[i] == invalid_hash) {
      interface.print(F("!%"));
      invalid_error = true;
    } else 
      for (uint8_t j = 0; j < i; j++) 
        if (valid_codes_[i] == valid_codes_[j]) {
          interface.print("!!");
          hash_crash = true;
          break;
        }
    interface.print(F("\t\t0x"));
    interface.print(long(callers_[i]), HEX);
    interface.println();
    interface.flush();
  }
  if (unknown_error) 
    interface.println(F(" **ERROR** Tried to register ukwnonk tokens. (!*)"));
  if (invalid_error) 
    interface.println(F(" **ERROR** Tried to register invalid commands. (!%)"));
  if (hash_crash) 
    interface.println(F(" **ERROR** Hash crashes found. (!!)"));

  #if SCPI_MAX_SPECIAL_COMMANDS
  hash_crash = false;
  unknown_error = false;
  invalid_error = false;
  interface.println();
  interface.print(F("VALID SPECIAL CODES : "));
  interface.print(special_codes_size_);
  interface.print(F(" / "));
  interface.print(max_special_commands);
  interface.println(F(" (SCPI_MAX_SPECIAL_COMMANDS)"));
  if (setup_errors.special_command_overflow) 
    interface.println(F(" **ERROR** Max special commands exceeded."));
  interface.println(F("  #\tHash\t\tHandler"));
  for (uint8_t i = 0; i < special_codes_size_; i++) {
    interface.print(F("  "));
    interface.print(i+1);
    interface.print(F(":\t"));
    interface.print(valid_special_codes_[i], HEX);
    if (valid_special_codes_[i] == unknown_hash) {
      interface.print(F("!*"));
      unknown_error = true;
    } else if (valid_special_codes_[i] == invalid_hash) {
      interface.print(F("!%"));
      invalid_error = true;
    } else
      for (uint8_t j = 0; j < i; j++)
        if (valid_special_codes_[i] == valid_special_codes_[j]) {
          interface.print("!!");
          hash_crash = true;
          break;
        }
    interface.print(F("\t\t0x"));
    interface.print(long(special_callers_[i]), HEX);
    interface.println();
    interface.flush();
  }
  if (unknown_error) 
    interface.println(F(" **ERROR** Tried to register ukwnonk tokens. (!*)"));
  if (invalid_error) 
    interface.println(F(" **ERROR** Tried to register invalid commands. (!%)"));
  if (hash_crash) 
    interface.println(F(" **ERROR** Hash crashes found. (!!)"));
  #endif
  
  interface.println(F("\nHASH Configuration:"));
  interface.print(F("  Hash size: "));
  interface.print(sizeof(scpi_hash_t)*8);
  interface.println(F("bits (SCPI_HASH_TYPE)"));
  interface.print(F("  Hash magic number: "));
  interface.println(hash_magic_number);
  interface.print(F("  Hash magic offset: "));
  interface.println(hash_magic_offset);
  interface.println(F("\n*******************\n"));
}
