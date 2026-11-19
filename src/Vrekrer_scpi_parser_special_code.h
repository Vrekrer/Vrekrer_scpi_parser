// This file is included in Vrekrer_scpi_parser.h 
// This allows Arduino IDE users to configure options with #define directives 
// Do not include Vrekrer_scpi_parser.h here

#if SCPI_MAX_SPECIAL_COMMANDS

void DoNothing(SCPI_C, Stream&) {}

/*!
 Registers a special command and its callback.
 If registration fails and space remains, stores an invalid entry with DoNothing as its callback.
 @param command  New valid command.
 @param caller  Function to call when the special command is received.
*/
void SCPI_Parser::RegisterSpecialCommand(char* command, 
                                         SCPI_special_caller_t caller) {
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken_(command_tokens[i]);
  const scpi_hash_t code = this->GetCommandCode_(command_tokens);

  const int branch_length = tree_length_ + command_tokens.Size();
  const bool branch_too_long =
      command_tokens.overflow_error
      or (branch_length > command_tokens.storage_size);
  setup_errors.branch_overflow |= branch_too_long;

  if (special_codes_size_ >= max_special_commands) {
    setup_errors.special_command_overflow = true;
    return;
  }

  const bool registration_error =
      branch_too_long
      or (code == invalid_hash)
      or (code == unknown_hash);

  if (registration_error) {
    valid_special_codes_[special_codes_size_] = invalid_hash;
    special_callers_[special_codes_size_] = DoNothing;
  } else {
    valid_special_codes_[special_codes_size_] = code;
    special_callers_[special_codes_size_] = caller;
  }
  special_codes_size_++;
}

/*!
 RegisterSpecialCommand version with RAM string support.

 Example:  
  ``my_instrument.RegisterSpecialCommand("GET:DATA", &getData);``  
 For lower RAM usage use the Flash strings version.
*/
void SCPI_Parser::RegisterSpecialCommand(const char* command, 
                                         SCPI_special_caller_t caller) {
  strcpy(msg_buffer_, command);
  this->RegisterSpecialCommand(msg_buffer_, caller);
}

/*!
 RegisterSpecialCommand version with Flash strings (F() macro) support.

 Example:  
  ``my_instrument.RegisterSpecialCommand("GET:DATA", &getData);``
*/
void SCPI_Parser::RegisterSpecialCommand(const __FlashStringHelper* command, 
                                         SCPI_special_caller_t caller) {
  strcpy_P(msg_buffer_, (const char *) command);
  this->RegisterSpecialCommand(msg_buffer_, caller);
}


#endif
