#ifndef PSPAMM_LEXER_H_
#define PSPAMM_LEXER_H_

#include <cctype>
#include <cstdint>
#include <string>
#include <istream>
#include <unordered_map>

enum Token {
  tok_eof = -1,
  tok_fn = -2,
  tok_var = -3,
  tok_identifier = -4,
  tok_number = -5,
  tok_for = -6
};

class Lexer {
private:
  std::string identifier;
  int64_t number;
  char lastChar = ' ';
  std::istream& in;

public:
  static const std::unordered_map<std::string,int64_t> String2Token;

  explicit Lexer(std::istream& in)
    : in(in) {}

  int64_t getToken();
  int64_t getNumber() const { return number; }
  std::string getIdentifier() const { return identifier; }
};

#endif // PSPAMM_LEXER_H_
