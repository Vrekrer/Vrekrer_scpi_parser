#ifndef VREKRER_SCPI_PARSER_CONF_H_
#define VREKRER_SCPI_PARSER_CONF_H_


/** \def SCPI_ARRAY_SYZE
 *  \brief Maximum size of commnad tree and number of parameters.
 * 
 *  Detailed explenation goes here.
 */
#define SCPI_ARRAY_SYZE 6


/** \def SCPI_MAX_TOKENS
 *  \brief Maximum number of valid tokens.
 * 
 *  Tokens are words (strings) used to form a command.
 *  e.g.
 *   "SYSTEM", "syst", "*IDN", "TIMER", "SET"
 *  Tokens are not case sesitive.
 *  Tokens usualy have a long form (complete word) and a short form (partial word).
 *  To define valid tokens we use upper case for the short form and lower case to
 *  complete the long form (if needed).
 *  e.g.
 *   "SYSTem"
 *   Sort form : "SYST" 
 *   Long form : "system"
 */
#define SCPI_MAX_TOKENS 15


/** \def SCPI_MAX_COMMANDS
 *  \brief Maximum number of registered commands.
 * 
 *  Commands are one or more tokens separeted by the ':' character.
 *  e.g.
 *   "*IDN?", "system:timer:set"
 *  A command terminated in the '?' character is defined as a "query".
 *  Inside a message commands are separated by the ';' character.
 */
#define SCPI_MAX_COMMANDS 20


/** \def SCPI_BUFFER_LENGTH
 *  \brief Length of the message buffer (chars).
 * 
 *  The longest command must fit into the buffer, don't forget an additional char
 *  for the string termination '\0' is needed.
 */
#define SCPI_BUFFER_LENGTH 64


/** \def SCPI_TIMEOUT
 *  \brief Timeout, in miliseconds, for GetMessage and ProcessInput.
 * 
 *  The receiving routine will wait SCPI_TIMEOUT for receiving the next char.
 *  If it's not received in time, the buffer was cleared and the message is lost.
 */
#define SCPI_TIMEOUT 10


/** \def SCPI_TERMINATOR
 *  \brief A string formed by any characters that indicates the end of a message.
 * 
 *  Usual termination chars are LineFeed '\n' and/or CarrierReturn '\r'.
 *  e.g.
 *   "\n"
 *   "\r\n"
 */
#define SCPI_TERMINATOR "\r"


#endif //VREKRER_SCPI_PARSER_CONF_H_