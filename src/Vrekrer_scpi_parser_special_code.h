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
  const CommandCode result = PrepareCommand_(command);

  if (special_codes_size_ >= max_special_commands) {
    setup_errors.special_command_overflow = true;
    return;
  }

  if (result.invalid) {
    valid_special_codes_[special_codes_size_] = invalid_hash;
    special_callers_[special_codes_size_] = DoNothing;
  } else {
    valid_special_codes_[special_codes_size_] = result.code;
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
