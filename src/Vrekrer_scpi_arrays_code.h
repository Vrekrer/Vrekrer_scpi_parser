// This file is included in Vrekrer_scpi_parser.h 
// This allows Arduino IDE users to configure options with #define directives 
// Do not include Vrekrer_scpi_parser.h here

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

///Dummy constructor.
SCPI_Commands::SCPI_Commands(){}

/*!
 Constructor that extracts and tokenize a command from a message.  
 @param message  Message to process.

 The message is processed until a space, tab or the end of the string is 
 found, the rest is available at not_processed_message.  
 The processed part is split on the ':' characters, the resulting parts 
 (tokens) are stored in the array.
*/
SCPI_Commands::SCPI_Commands(char* message) {
  char* token = message;
  // Trim leading spaces and tabs
  while (isspace(*token)) token++;
  // Save parameters and multicommands for later
  not_processed_message = strpbrk(token, " \t");
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

/// Dummy constructor.
SCPI_Parameters::SCPI_Parameters(){}


/*!
 Constructor that extracts and splits parameters from a message.  
 @param message[in,out]  Message to process.

 The message is split on the ',' characters, the resulting parts 
 (parameters) are stored in the array after trimming any start spaces.
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
  //TODO add support for strings parameters (do not split parameters inside "")
}
