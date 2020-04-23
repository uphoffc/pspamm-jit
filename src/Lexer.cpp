#include "Lexer.h"
#include <sstream>

const std::unordered_map<std::string,int64_t> Lexer::String2Token{
  {"fn", tok_fn},
  {"var", tok_var},
  {"identifier", tok_identifier},
  {"number", tok_number},
  {"for", tok_for}
};

int64_t Lexer::getToken() {
  in.peek();
  if (!in.good()) {
    return tok_eof;
  }

  while (isspace(lastChar)) {
    in.get(lastChar);
  }

  if (isalpha(lastChar)) {
    identifier.clear();
    do {
      identifier += lastChar;
      in.get(lastChar);
    } while (isalnum(lastChar));

    if (String2Token.find(identifier) != String2Token.end()) {
      return String2Token.at(identifier);
    }
    return tok_identifier;
  }

  if (isdigit(lastChar)) {
    std::stringstream value;
    do {
      value << lastChar;
      in.get(lastChar);
    } while (isdigit(lastChar));
    value >> number;
    return tok_number;
  }

  if (lastChar == '#') {
    do {
      in.get(lastChar);
    } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

    return getToken();
  }

  int thisChar = lastChar;
  in.get(lastChar);
  return thisChar;
}
