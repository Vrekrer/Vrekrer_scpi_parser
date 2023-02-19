// This file is included in Vrekrer_scpi_parser.h 
// This allows Arduino IDE users to configure options with #define directives 
// Do not include Vrekrer_scpi_parser.h here

#if SCPI_MAX_SPECIAL_COMMANDS

/*!
 Registers a new valid special command and associate a procedure to it.
 @param command  New valid command.
 @param caller  Procedure associated to the valid command.
*/
void SCPI_Parser::RegisterSpecialCommand(char* command, 
                                         SCPI_special_caller_t caller) {
  if (special_codes_size_ >= max_special_commands) {
    setup_errors.special_command_overflow = true;
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
  setup_errors.branch_overflow |= overflow_error;
  if (overflow_error) code = invalid_hash;

  valid_special_codes_[special_codes_size_] = code;
  special_callers_[special_codes_size_] = caller;
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