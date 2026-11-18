// Host tests for Vrekrer_scpi_parser.
//
// The library is compiled for a PC using the small Arduino.h shim in this
// folder. Build and run once per hash size, for example:
//
//   c++ -std=c++11 -Wall -I extras/test -I src -DSCPI_HASH_TYPE=uint8_t \
//       -DTEST_SETUP_ERRORS_API extras/test/test_parser.cpp -o test_parser
//
// Repeat with uint16_t and uint32_t. See README.md in this folder.

#ifndef SCPI_MAX_TOKENS
  #define SCPI_MAX_TOKENS 48
#endif
#ifndef SCPI_MAX_COMMANDS
  #define SCPI_MAX_COMMANDS 60
#endif
#ifndef SCPI_MAX_SPECIAL_COMMANDS
  #define SCPI_MAX_SPECIAL_COMMANDS 2
#endif

#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"

#include <vector>

unsigned long host_millis = 0;

static int checks = 0;
static int failures = 0;

#define CHECK(condition)                                               \
  do {                                                                 \
    checks++;                                                          \
    if (not (condition)) {                                             \
      failures++;                                                      \
      printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);    \
    }                                                                  \
  } while (0)

// Handlers append a tag to `called`, so a test can see what was dispatched.
static std::string called;

template <int N>
void Tagged(SCPI_C, SCPI_P, Stream&) {
  called += std::to_string(N) + ",";
}
static void HandlerError(SCPI_C, SCPI_P, Stream&) { called += "E,"; }
static void HandlerSpecial(SCPI_C, Stream&) { called += "S,"; }

#define TAG(n) &Tagged<n>
static const SCPI_caller_t tagged[] = {
  TAG(0),  TAG(1),  TAG(2),  TAG(3),  TAG(4),  TAG(5),  TAG(6),  TAG(7),
  TAG(8),  TAG(9),  TAG(10), TAG(11), TAG(12), TAG(13), TAG(14), TAG(15),
  TAG(16), TAG(17), TAG(18), TAG(19), TAG(20), TAG(21), TAG(22), TAG(23),
  TAG(24), TAG(25), TAG(26), TAG(27), TAG(28), TAG(29), TAG(30), TAG(31),
};
static std::string Tag(int n) { return std::to_string(n) + ","; }

// Parser with access to the token list, used to search for commands with a
// given hash. The reference hash below follows the documented algorithm and
// does not use the library's own hashing code.
class Probe : public SCPI_Parser {
 public:
  Probe() { SetErrorHandler(&HandlerError); }

  scpi_hash_t ReferenceHash(const std::vector<uint8_t>& sequence,
                            bool query) const {
    scpi_hash_t code = hash_magic_offset;
    for (uint8_t j : sequence) code = scpi_hash_t(code * hash_magic_number + j);
    if (query) code = scpi_hash_t(code * hash_magic_number - 1);
    return code;
  }

  // Look for a command of `depth` known keywords whose hash is `target`.
  bool FindCommand(scpi_hash_t target, uint8_t depth, bool query,
                   std::string& command) const {
    std::vector<uint8_t> sequence(depth, 0);
    while (true) {
      bool usable = true;
      for (uint8_t j : sequence) usable = usable and (tokens_[j][0] != '*');
      if (usable and ReferenceHash(sequence, query) == target) {
        command.clear();
        for (uint8_t i = 0; i < depth; i++) {
          if (i > 0) command += ":";
          command += tokens_[sequence[i]];
        }
        if (query) command += "?";
        return true;
      }
      uint8_t k = 0;
      while ((k < depth) and (++sequence[k] == tokens_size_)) {
        sequence[k] = 0;
        k++;
      }
      if (k == depth) return false;
    }
  }

  bool FindCommand(scpi_hash_t target, bool query, std::string& command) const {
    for (uint8_t depth = 2; depth <= 3; depth++)
      if (FindCommand(target, depth, query, command)) return true;
    return false;
  }
};

static void Run(SCPI_Parser& parser, const std::string& message) {
  Stream interface;
  std::vector<char> buffer(message.begin(), message.end());
  buffer.push_back('\0');
  called.clear();
  parser.Execute(buffer.data(), interface);
}

static std::string DebugInfo(SCPI_Parser& parser) {
  Stream interface;
  parser.PrintDebugInfo(interface);
  return interface.output;
}

// Single keyword commands, used to fill the token list.
static void RegisterFillers(SCPI_Parser& parser) {
  const char* fillers[] = {"SYSTem", "MEASure", "VOLTage", "CURRent", "OUTPut",
                           "STATe", "TRIGger", "DELay", "SOURce", "RANGe"};
  for (const char* filler : fillers) parser.RegisterCommand(filler, tagged[0]);
}

struct Entry {
  const char* tree;     // TreeBase used when registering
  const char* command;  // Command as registered
  const char* message;  // Message that must reach it
};

// Command tables from the library examples. They must register without
// setup errors and every command must reach its own handler.
static void CheckTable(const char* name, const Entry* table, int size) {
  Probe parser;
  const char* tree = "";
  for (int i = 0; i < size; i++) {
    if (strcmp(tree, table[i].tree) != 0) {
      tree = table[i].tree;
      parser.SetCommandTreeBase(tree);
    }
    parser.RegisterCommand(table[i].command, tagged[i]);
  }
  std::string debug = DebugInfo(parser);
  bool clean = (debug.find("ERROR") == std::string::npos);
  if (not clean) printf("  %s debug info:\n%s", name, debug.c_str());
  CHECK(clean);
#ifdef TEST_SETUP_ERRORS_API
  CHECK(not parser.HasSetupError());
#endif
  for (int i = 0; i < size; i++) {
    Run(parser, table[i].message);
    if (called != Tag(i))
      printf("  %s: '%s' called '%s'\n", name, table[i].message, called.c_str());
    CHECK(called == Tag(i));
  }
}

static void TestExampleTables() {
  printf("example command tables\n");
  const Entry configuration_options[] = {
    {"STATus:OPERation", ":CONDition?", "STAT:OPER:COND?"},
    {"STATus:OPERation", ":ENABle", "STATus:OPERation:ENABle"},
    {"STATus:OPERation", ":EVENt?", "stat:oper:even?"},
    {"STATus:QUEStionable", ":CONDition?", "STAT:QUES:COND?"},
    {"STATus:QUEStionable", ":ENABle", "STAT:QUES:ENAB"},
    {"STATus:QUEStionable", ":EVENt?", "STAT:QUES:EVEN?"},
    {"STATus", ":OPERation?", "STAT:OPER?"},
    {"STATus", ":QUEStionable?", "STAT:QUES?"},
    {"STATus", ":PRESet", "STAT:PRES"},
    {"SYSTem", ":ERRor?", "SYST:ERR?"},
    {"SYSTem", ":ERRor:NEXT?", "SYST:ERR:NEXT?"},
    {"SYSTem", ":VERSion?", "SYST:VERS?"},
    {"", "*CLS", "*CLS"},
    {"", "*ESE", "*ESE"},
    {"", "*ESE?", "*ESE?"},
    {"", "*ESR", "*ESR"},
    {"", "*IDN?", "*IDN?"},
    {"", "*OPC", "*OPC"},
    {"", "*OPC?", "*OPC?"},
    {"", "*RST", "*RST"},
    {"", "*SRE", "*SRE"},
    {"", "*SRE?", "*SRE?"},
    {"", "*STB", "*STB"},
    {"", "*TST?", "*TST?"},
    {"", "*WAI", "*WAI"},
  };
  CheckTable("Configuration_Options", configuration_options, 25);

  const Entry error_handling[] = {
    {"", "*IDN?", "*IDN?"},
    {"", "SYSTem:ERRor?", "SYST:ERR?"},
  };
  CheckTable("Error_Handling", error_handling, 2);

  const Entry numeric_suffixes[] = {
    {"", "*IDN?", "*IDN?"},
    {"", "DIn#?", "DI3?"},
    {"", "DOut#", "DOUT12"},
    {"", "DOut#?", "DO7?"},
  };
  CheckTable("Numeric_suffixes", numeric_suffixes, 4);

  const Entry raw_data_parameters[] = {
    {"", "*IDN?", "*IDN?"},
    {"", "ECHO:NORMal", "ECHO:NORM"},
    {"", "ECHO:NORMal?", "ECHO:NORM?"},
  };
  CheckTable("RawData_Parameters", raw_data_parameters, 3);

  const Entry scpi_dimmer[] = {
    {"", "*IDN?", "*IDN?"},
    {"SYSTem:LED", ":BRIGhtness", "SYST:LED:BRIG 5"},
    {"SYSTem:LED", ":BRIGhtness?", "SYST:LED:BRIG?"},
    {"SYSTem:LED", ":BRIGhtness:INCrease", "SYST:LED:BRIG:INC"},
    {"SYSTem:LED", ":BRIGhtness:DECrease", "SYST:LED:BRIG:DEC"},
  };
  CheckTable("SCPI_Dimmer", scpi_dimmer, 5);
}

static void TestSpecialCommand() {
  printf("special command dispatch\n");
  Probe parser;
  parser.RegisterCommand("*IDN?", tagged[1]);
  parser.RegisterSpecialCommand("DATA:RAW", &HandlerSpecial);
  Stream interface;
  interface.input = "DATA:RAW 0123\n";
  called.clear();
  parser.ProcessInput(interface, "\n");
  CHECK(called == "S,");
}

// A command whose hash is 0 (the value used for unknown commands) used to be
// stored as invalid and could never be executed.
static void TestCommandHashingToUnknownValue() {
  printf("command hashing to the unknown-command value\n");
  Probe parser;
  RegisterFillers(parser);
  std::string command;
  CHECK(parser.FindCommand(0, true, command));
  parser.RegisterCommand(command.c_str(), tagged[1]);
  Run(parser, command);
  if (called != Tag(1))
    printf("  '%s' called '%s'\n", command.c_str(), called.c_str());
  CHECK(called == Tag(1));
}

// A command whose hash is 1 (the value stored for failed registrations) used
// to run the handler of an earlier failed registration.
static void TestCommandHashingToInvalidValue() {
  printf("command hashing to the invalid-command value\n");
  Probe parser;
  // Too many keywords for SCPI_ARRAY_SYZE (6), so this registration fails.
  parser.RegisterCommand("SYSTem:MEASure:VOLTage:CURRent:OUTPut:STATe:TRIGger",
                         tagged[2]);
  RegisterFillers(parser);
  std::string command;
  CHECK(parser.FindCommand(1, true, command));
  parser.RegisterCommand(command.c_str(), tagged[1]);
  Run(parser, command);
  if (called != Tag(1))
    printf("  '%s' called '%s'\n", command.c_str(), called.c_str());
  CHECK(called == Tag(1));
}

// A TreeBase whose hash was 0 was taken as the root, and one whose hash was 1
// made every command registered under it invalid.
static void TestTreeBaseHashingToReservedValues() {
  for (scpi_hash_t target = 0; target <= 1; target++) {
    printf("TreeBase hashing to %d\n", int(target));
    Probe parser;
    RegisterFillers(parser);
    std::string tree;
    CHECK(parser.FindCommand(target, false, tree));
    parser.SetCommandTreeBase(tree.c_str());
    parser.RegisterCommand(":LEAF?", tagged[1]);
    parser.SetCommandTreeBase("");
    Run(parser, tree + ":LEAF?");
    if (called != Tag(1))
      printf("  '%s:LEAF?' called '%s'\n", tree.c_str(), called.c_str());
    CHECK(called == Tag(1));
    Run(parser, "LEAF?");
    CHECK(called.find(Tag(1)) == std::string::npos);
  }
}

// With the old default magic number (37), any table with more than 37
// tokens has structural collisions, e.g. tokens (0, 37) and (1, 0):
// (7*37 + 0)*37 + 37 == (7*37 + 1)*37 + 0.
static void TestManyTokens() {
  printf("more than 37 tokens\n");
  Probe parser;
  char name[3] = {0, 0, 0};
  for (int i = 0; i < 40; i++) {
    name[0] = char('A' + i / 26);
    name[1] = char('A' + i % 26);
    parser.RegisterCommand(name, tagged[0]);
  }
  parser.RegisterCommand("AA:BL", tagged[1]);  // tokens 0, 37
  parser.RegisterCommand("AB:AA", tagged[2]);  // tokens 1, 0
#ifdef TEST_SETUP_ERRORS_API
  CHECK(not parser.HasSetupError());
#endif
  Run(parser, "AA:BL");
  CHECK(called == Tag(1));
  Run(parser, "AB:AA");
  CHECK(called == Tag(2));
}

static void TestDefaultMagicNumbers() {
  printf("default magic numbers\n");
  SCPI_Parser parser;
  if (sizeof(scpi_hash_t) < 4) {
    // Unchanged for 8 and 16 bit hashes.
    CHECK(parser.hash_magic_number == 37);
  } else {
    CHECK(parser.hash_magic_number == scpi_hash_t(16777619UL));
  }
  CHECK(parser.hash_magic_offset == 7);
}

#ifdef TEST_SETUP_ERRORS_API
static void TestSetupErrors() {
  printf("setup errors\n");
  {
    Probe parser;
    parser.RegisterCommand("*IDN?", tagged[1]);
    CHECK(not parser.HasSetupError());
    SCPI_Parser::SetupErrors errors = parser.GetSetupErrors();
    CHECK(not errors.hash_collision);
    CHECK(not errors.ambiguous_token);
  }
  {
    // The same command registered twice.
    Probe parser;
    parser.RegisterCommand("MEASure:VOLTage?", tagged[1]);
    parser.RegisterCommand("MEASure:VOLTage?", tagged[2]);
    CHECK(parser.HasSetupError());
    CHECK(parser.GetSetupErrors().hash_collision);
    CHECK(DebugInfo(parser).find("!!") != std::string::npos);
  }
  {
    Probe parser;
    parser.RegisterSpecialCommand("DATA:RAW", &HandlerSpecial);
    parser.RegisterSpecialCommand("DATA:RAW", &HandlerSpecial);
    CHECK(parser.GetSetupErrors().hash_collision);
  }
  {
    // Two tokens with the same short form: STAT can only reach one of them.
    Probe parser;
    parser.RegisterCommand("STATus?", tagged[1]);
    parser.RegisterCommand("STATe", tagged[2]);
    CHECK(parser.HasSetupError());
    CHECK(parser.GetSetupErrors().ambiguous_token);
    CHECK(not parser.GetSetupErrors().hash_collision);
    CHECK(DebugInfo(parser).find("same short or long form") != std::string::npos);
  }
  {
    // The long form of one token is the short form of the other.
    Probe parser;
    parser.RegisterCommand("LEVel", tagged[1]);
    parser.RegisterCommand("LEVELs", tagged[2]);
    CHECK(parser.GetSetupErrors().ambiguous_token);
  }
  {
    // Same token with a different case.
    Probe parser;
    parser.RegisterCommand("RANGe", tagged[1]);
    parser.RegisterCommand("RANGE?", tagged[2]);
    CHECK(parser.GetSetupErrors().ambiguous_token);
  }
  {
    // A query and a command reuse the same token: not ambiguous.
    Probe parser;
    parser.RegisterCommand("STATus?", tagged[1]);
    parser.RegisterCommand("STATus", tagged[2]);
    CHECK(not parser.HasSetupError());
  }
  {
    // A token followed by the same token with numeric suffix: both work.
    Probe parser;
    parser.RegisterCommand("CHANnel?", tagged[1]);
    parser.RegisterCommand("CHANnel#?", tagged[2]);
    CHECK(not parser.HasSetupError());
    Run(parser, "CHAN?");
    CHECK(called == Tag(1));
    Run(parser, "CHAN2?");
    CHECK(called == Tag(2));
  }
  {
    // In the opposite order, CHAN always selects the numeric suffix token.
    Probe parser;
    parser.RegisterCommand("CHANnel#?", tagged[1]);
    parser.RegisterCommand("CHANnel?", tagged[2]);
    CHECK(parser.GetSetupErrors().ambiguous_token);
  }
  {
    Probe parser;
    for (int i = 0; i <= SCPI_MAX_COMMANDS; i++)
      parser.RegisterCommand("*IDN?", tagged[1]);
    CHECK(parser.GetSetupErrors().command_overflow);
  }
  if (sizeof(scpi_hash_t) == 1) {
    // These two commands have the same 8 bit hash with the default settings.
    Probe parser;
    parser.RegisterCommand("SOURce:VOLTage", tagged[1]);
    parser.RegisterCommand("TRIGger:SOURce?", tagged[2]);
    CHECK(not parser.HasSetupError());
    parser.RegisterCommand("TRIGger:DELay", tagged[3]);
    CHECK(parser.GetSetupErrors().hash_collision);
  }
}
#endif

int main() {
  printf("Vrekrer SCPI parser host tests, %d bit hash\n",
         int(sizeof(scpi_hash_t) * 8));
  TestExampleTables();
  TestSpecialCommand();
  if (sizeof(scpi_hash_t) == 1) {
    // Commands that hash to 0 or 1 are rare with wider hashes, but they are
    // easy to find with 8 bits.
    TestCommandHashingToUnknownValue();
    TestCommandHashingToInvalidValue();
    TestTreeBaseHashingToReservedValues();
  }
  if (sizeof(scpi_hash_t) >= 4) TestManyTokens();
  TestDefaultMagicNumbers();
#ifdef TEST_SETUP_ERRORS_API
  TestSetupErrors();
#endif
  printf("%d checks, %d failures\n", checks, failures);
  return failures ? 1 : 0;
}
