// This file is included in Vrekrer_scpi_parser.h 
// This allows Arduino IDE users to configure options with #define directives 
// Do not include Vrekrer_scpi_parser.h here

static constexpr char scpi_header_separators[] = " \t\r";

// ## SCPI_String_Array member functions ##

///Add indexing capability.
char* SCPI_String_Array::operator[](const uint8_t index) const {
  if (index >= size_) return NULL; //Invalid index
  return values_[index];
}

///Append new string (LIFO stack Push).
void SCPI_String_Array::Append(char* value) {
  overflow_error = (size_ >= storage_size);
  if (overflow_error) return;
  values_[size_] = value;
  size_++;
}

///LIFO stack Pop
char* SCPI_String_Array::Pop() {
  if (size_ == 0) return NULL; //Empty array
  size_--;
  return values_[size_];
}

///Returns the first element of the array
char* SCPI_String_Array::First() const {
  if (size_ == 0) return NULL; //Empty array
  return values_[0];
}

///Returns the last element of the array
char* SCPI_String_Array::Last() const {
  if (size_ == 0) return NULL; //Empty array
  return values_[size_ - 1];
}

///Array size
uint8_t SCPI_String_Array::Size() const {
  return size_;
}


// ## SCPI_Commands member functions ##

/// Default constructor.
SCPI_Commands::SCPI_Commands(){}

/*!
 Extracts the command header and splits it into tokens.
 @param message  Message to process.

 The header ends at the first space, tab, carriage return, or end of the string.
 The remaining text is available through not_processed_message.
 The header is split at colons, and the tokens are stored in the array.
*/
SCPI_Commands::SCPI_Commands(char* message) {
  char* token = message;
  // Skip leading whitespace.
  while (isspace(*token)) token++;
  // Keep a pointer to the rest of the message.
  not_processed_message = strpbrk(token, scpi_header_separators);
  if (not_processed_message != NULL) {
   not_processed_message[0] = '\0';
   not_processed_message++;
  }
  // Split using ':'
  token = strtok(token, ":");
  while (token != NULL) {
    this->Append(token);
    token = strtok(NULL, ":");
  }
}

// ## SCPI_Parameters member functions ##

/// Default constructor.
SCPI_Parameters::SCPI_Parameters(){}


/*!
 Constructor that extracts and splits parameters from a message.  
 @param message[in,out]  Message to process.

 The message is split at commas. Leading whitespace is removed from each
 parameter before it is stored in the array.
*/
SCPI_Parameters::SCPI_Parameters(char* message) {
  char* parameter = message;
  // Split using ','
  parameter = strtok(parameter, ",");
  while (parameter != NULL) {
    while(isspace(*parameter)) parameter++;
    this->Append(parameter);
    parameter = strtok(NULL, ",");
  }
  // TODO: Preserve commas inside quoted string parameters.
}
