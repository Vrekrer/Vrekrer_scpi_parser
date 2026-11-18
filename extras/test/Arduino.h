// Minimal Arduino API shim so the parser can be compiled and tested on a PC.
// Only what Vrekrer_scpi_parser uses is provided.
#ifndef ARDUINO_H_HOST_SHIM
#define ARDUINO_H_HOST_SHIM

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <type_traits>

typedef uint8_t byte;

#define DEC 10
#define HEX 16

class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper*>(string_literal))
#define strcpy_P strcpy

extern unsigned long host_millis;
inline unsigned long millis() { return host_millis; }

class String {
 public:
  String(const char* value = "") : value_(value) {}
  const char* c_str() const { return value_.c_str(); }
 private:
  std::string value_;
};

// Stream that reads from `input` and appends everything printed to `output`.
class Stream {
 public:
  std::string input;
  std::string output;

  int available() { return int(input.size() - read_pos_); }
  int read() { return (read_pos_ < input.size()) ? input[read_pos_++] : -1; }
  void flush() {}

  size_t print(const char* value) { output += value; return strlen(value); }
  size_t print(const __FlashStringHelper* value) {
    return print(reinterpret_cast<const char*>(value));
  }
  size_t print(const String& value) { return print(value.c_str()); }
  size_t print(char value) { output += value; return 1; }
  template <typename T,
            typename = typename std::enable_if<std::is_integral<T>::value>::type>
  size_t print(T value, int base = DEC) {
    char buffer[32];
    unsigned long long magnitude = (unsigned long long)value;
    if (std::is_signed<T>::value and value < 0) {
      output += '-';
      magnitude = (unsigned long long)(-(long long)value);
    }
    int length = 0;
    do {
      int digit = int(magnitude % base);
      buffer[length++] = char(digit < 10 ? '0' + digit : 'A' + digit - 10);
      magnitude /= base;
    } while (magnitude > 0);
    for (int i = length - 1; i >= 0; i--) output += buffer[i];
    return length;
  }

  size_t println() { output += "\r\n"; return 2; }
  template <typename... Args>
  size_t println(Args... args) {
    size_t n = print(args...);
    return n + println();
  }

 private:
  size_t read_pos_ = 0;
};

#endif  // ARDUINO_H_HOST_SHIM
