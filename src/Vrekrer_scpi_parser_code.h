// This file is included in Vrekrer_scpi_parser.h 
// This allows Arduino IDE users to configure options with #define directives 
// Do not include Vrekrer_scpi_parser.h here.


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
  if (tokens_size_ >= max_tokens) {
    setup_errors.token_overflow = true;
    return;
  }
  size_t token_size = strlen(token);
  //Remove query symbols
  if (token[token_size - 1] == '?') token_size--;
  for (uint8_t i = 0; i < tokens_size_; i++)
    //Check if the token is allready added
    if ( (strncmp(token, tokens_[i], token_size) == 0) 
          and (token_size == strlen(tokens_[i])) ) return;
  char *stored_token = new char [token_size + 1];
  strncpy(stored_token, token, token_size);
  stored_token[token_size] = '\0';
  tokens_[tokens_size_] = stored_token;
  tokens_size_++;
}

/*!
 Get a hash from a valid command
 @param commands  Keywords of a command
 @return hash

 Return ``unknown_hash`` if the command contains  
 keywords not registered as tokens.  
 The hash is calculated including the TreeBase hash.  
 @see SetCommandTreeBase
*/
scpi_hash_t SCPI_Parser::GetCommandCode_(SCPI_Commands& commands) {
  if (tree_code_ == invalid_hash) return invalid_hash;
  scpi_hash_t code;
  code = (tree_code_ == 0) ? hash_magic_offset : tree_code_;
  if (commands.Size()==0) return unknown_hash;
  //Loop all keywords in the command
  for (uint8_t i = 0; i < commands.Size(); i++) {
    //Get keywords's length
    size_t header_length = strlen(commands[i]);
    //For the last keyword remove the query symbol if needed
    bool is_query = false;
    if (i == commands.Size() - 1) {
      is_query = (commands[i][header_length - 1] == '?');
      if (is_query) header_length--;
    }

    //Loop over all the known tokens
    for (uint8_t j = 0; j < tokens_size_; j++) {
      //Get the token's short and long lengths
      size_t short_length = 0;
      while (isupper(tokens_[j][short_length])) short_length++;
      size_t long_length = strlen(tokens_[j]);

      //If the token allows numeric suffixes
      //remove the trailing digits from the keyword
      if ( (tokens_[j][long_length - 1] == '#')
         and (commands[i][header_length - 1] != '#') ) {
        long_length--;
        while (isdigit(commands[i][header_length - 1])) header_length--;
      }

      //Test if the keyword match with the token
      //otherwise test next token
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
      //If the keyword does not match any token return unknown_hash
      if (j == (tokens_size_ - 1)) return unknown_hash;
    }
    //If last keyword is a query, add a hashing step
    if (is_query) {
      code *= hash_magic_number;
      code -= 1;
    }
  }
  return code;
}

/*!
 Change the TreeBase for the next RegisterCommand calls.
 @param tree_base  TreeBase to be used.  
        An empty string ``""`` sets the TreeBase to root.
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

/*!
 Registers a new valid command and associate a procedure to it.
 @param command  New valid command.
 @param caller  Procedure associated to the valid command.
*/
void SCPI_Parser::RegisterCommand(char* command, SCPI_caller_t caller) {
  if (codes_size_ >= max_commands) {
    setup_errors.command_overflow = true;
    return;
  }
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken_(command_tokens[i]);
  scpi_hash_t code = this->GetCommandCode_(command_tokens);
  
  //Check for errors
  if (code == unknown_hash) code = invalid_hash;
  bool overflow_error = command_tokens.overflow_error;
  overflow_error |= (tree_length_+command_tokens.Size()) 
                    > command_tokens.storage_size;
  setup_errors.command_overflow |= overflow_error;
  if (overflow_error) code = invalid_hash;

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
    message = multicomands;
    SCPI_Parameters parameters(commands.not_processed_message);
    scpi_hash_t code = this->GetCommandCode_(commands);
    if (code == unknown_hash) {
      //Call ErrorHandler UnknownCommand
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
  A timeout occurs (SCPI_Parser::timeout ms without new chars) (default 10 ms)  
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
    
    #if SCPI_MAX_SPECIAL_COMMANDS
    //For the first space only.
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
      //restore original message.
      msg_buffer_[message_length_ - 1] = ' ';
      for (uint8_t i = 0; i < commands.Size()-1; i++)
        commands[i][strlen(commands[i])] = ':';
      commands.not_processed_message--;
      commands.not_processed_message[0] = ' ';
    }
    #endif

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

///Prints debug information to an interface.
void SCPI_Parser::PrintDebugInfo(Stream& interface) 
{
  interface.println(F("*** DEBUG INFO ***\n"));
  interface.print(F("Max command tree branches: "));
  interface.print(SCPI_ARRAY_SYZE);
  interface.println(F(" (SCPI_ARRAY_SYZE)"));
  if (setup_errors.branch_overflow) 
    interface.println(F(" **ERROR** Max branch size exceeded."));
  interface.print(F("Max number of parameters: "));
  interface.print(SCPI_ARRAY_SYZE);
  interface.println(F(" (SCPI_ARRAY_SYZE)"));
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
