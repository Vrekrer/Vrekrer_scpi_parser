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
  strncpy(tokens_storage_[tokens_size_], token, token_size);
  tokens_storage_[tokens_size_][token_size] = '\0';
  tokens_[tokens_size_] = tokens_storage_[tokens_size_];
  tokens_size_++;
}

/*!
 Calculates the hash of a command.
 @param commands  Keywords of the command.
 @return The command hash, token count, and an invalid flag. The flag is set
         if the command is empty or unrecognized, or the base path is invalid.

 The calculation starts with the TreeBase hash when a base is set.
 @see SetCommandTreeBase
*/
SCPI_Parser::CommandCode SCPI_Parser::GetCommandCode_(SCPI_Commands& commands) {
  const CommandCode invalid_command = {0, true, commands.Size()};
  if (tokens_size_ == 0  // No registered tokens to match.
      or commands.Size() == 0  // Empty command.
      or tree_code_.invalid) {
    return invalid_command;
  }

  // Start with the TreeBase hash. At the root, use the offset.
  scpi_hash_t code =
      (tree_code_.length == 0) ? hash_magic_offset : tree_code_.code;

  for (uint8_t i = 0; i < commands.Size(); i++) {
    const char* keyword = commands[i];
    size_t keyword_length = strlen(keyword);

    const bool is_query =
        (i + 1 == commands.Size())  // Last keyword only.
        and (keyword[keyword_length - 1] == '?');
    if (is_query) keyword_length--; // Ignore the trailing '?'
    if (keyword_length == 0) return invalid_command;

    // Find the first registered token that matches this keyword.
    uint8_t token_index = 0;
    for (; token_index < tokens_size_; token_index++) {
      const char* token = tokens_[token_index];
      size_t long_token_length = strlen(token);
      size_t short_token_length = 0;
      while (isupper(token[short_token_length])) short_token_length++;

      const bool is_numeric_sufix_token =
          (keyword[keyword_length - 1] != '#') //Used in command registration.
          and (token[long_token_length - 1] == '#');
      if (is_numeric_sufix_token) {
        // Ignore the token's trailing '#'.
        long_token_length--;
        // Ignore trailing digits in the keyword.
        while (isdigit(keyword[keyword_length - 1])) {
          keyword_length--;
          if (keyword_length == 0) return invalid_command;
        }
      }

      const bool keyword_matches_token_lengths =
          (keyword_length == short_token_length)
          or (keyword_length == long_token_length);
      if (keyword_matches_token_lengths
          and (strncasecmp(keyword, token, keyword_length) == 0)) {
        // Keep the matching token_index as the result of the search.
        break;
      }

      // Restore the keyword length before trying the next token.
      if (is_numeric_sufix_token) {
        keyword_length = strlen(keyword);
        if (is_query) keyword_length--;
      }
    }

    // Reached the end of the token list without a match.
    if (token_index == tokens_size_) return invalid_command;

    // Update the hash with the matched token index.
    code *= hash_magic_number;
    code += token_index;

    // Distinguish a query from a command with the same keywords.
    if (is_query) {
      code *= hash_magic_number;
      code -= 1;
    }
  }
  return {
    code,            // Calculated hash.
    false,           // The command is valid.
    commands.Size()  // Number of keywords.
  };
}

void SCPI_Parser::ResetCommandTreeBase_() {
  tree_code_ = {0, false, 0};
}

/*!
 Sets the base path for subsequent command registrations.
 @param tree_base  The base path. A path with no tokens selects the root.
*/
void SCPI_Parser::SetCommandTreeBase(char* tree_base) {
  ResetCommandTreeBase_();
  tree_code_ = PrepareCommand_(tree_base);
  if (tree_code_.length == 0) {
    ResetCommandTreeBase_();
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
  CommandCode result = this->GetCommandCode_(command_tokens);

  const int branch_length = tree_code_.length + result.length;
  const bool branch_too_long =
      command_tokens.overflow_error
      or (branch_length > command_tokens.storage_size);
  setup_errors.branch_overflow |= branch_too_long;

  result.invalid |= branch_too_long;
  return result;
}

/*!
 Registers a command and its callback.
 If registration fails and space remains, stores the callback in an invalid entry.
 @param command  New valid command.
 @param caller  Function to call when the command is received.
*/
void SCPI_Parser::RegisterCommand(char* command, SCPI_caller_t caller) {
  const CommandCode result = PrepareCommand_(command);

  if (codes_size_ >= max_commands) {
    setup_errors.command_overflow = true;
    return;
  }

  registered_command_codes_[codes_size_] = result.code;
  registered_command_validity_[codes_size_] = not result.invalid;
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

    ResetCommandTreeBase_();
    SCPI_Commands commands(message);
    message = next_command;
    SCPI_Parameters parameters(commands.not_processed_message);
    const CommandCode result = this->GetCommandCode_(commands);
    if (result.invalid) {
      // Report an unknown command.
      last_error = ErrorCode::UnknownCommand;
      (*callers_[max_commands])(commands, parameters, interface);
      continue;
    }
    for (uint8_t i = 0; i < codes_size_; i++)
      if (registered_command_validity_[i]
          and (registered_command_codes_[i] == result.code)) {
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
    msg_buffer_[message_length_] = '\0';

    #if SCPI_MAX_SPECIAL_COMMANDS
    // Check for a special command when the first header separator arrives.
    char* separator = strpbrk(msg_buffer_, scpi_header_separators);
    if (separator == &msg_buffer_[message_length_ - 1]) {
      const char separator_char = *separator;
      *separator = '\0';
      ResetCommandTreeBase_();
      SCPI_Commands commands(msg_buffer_);
      const CommandCode result = this->GetCommandCode_(commands);
      if (not result.invalid) {
        for (uint8_t i = 0; i < special_codes_size_; i++)
          if (special_command_validity_[i]
              and special_command_codes_[i] == result.code) {
            (*special_callers_[i])(commands, interface);
            message_length_ = 0;
            return msg_buffer_;
          }
      }
      // Restore the original message for normal command parsing.
      *separator = separator_char;
      for (uint8_t i = 0; i < commands.Size()-1; i++)
        commands[i][strlen(commands[i])] = ':';
    }
    #endif

    // Check for the message terminator.
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
    interface.print(registered_command_codes_[i], HEX);
    if (not registered_command_validity_[i]) {
      interface.print(F("!%"));
      invalid_error = true;
    } else 
      for (uint8_t j = 0; j < i; j++) 
        if (registered_command_validity_[j]
            and registered_command_codes_[i] == registered_command_codes_[j]) {
          interface.print("!!");
          hash_crash = true;
          break;
        }
    interface.print(F("\t\t0x"));
    interface.print(long(callers_[i]), HEX);
    interface.println();
    interface.flush();
  }
  if (invalid_error) 
    interface.println(F(" **ERROR** Tried to register invalid commands. (!%)"));
  if (hash_crash) 
    interface.println(F(" **ERROR** Hash crashes found. (!!)"));

  #if SCPI_MAX_SPECIAL_COMMANDS
  hash_crash = false;
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
    interface.print(special_command_codes_[i], HEX);
    if (not special_command_validity_[i]) {
      interface.print(F("!%"));
      invalid_error = true;
    } else
      for (uint8_t j = 0; j < i; j++)
        if (special_command_validity_[j]
            and special_command_codes_[i] == special_command_codes_[j]) {
          interface.print("!!");
          hash_crash = true;
          break;
        }
    interface.print(F("\t\t0x"));
    interface.print(long(special_callers_[i]), HEX);
    interface.println();
    interface.flush();
  }
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
