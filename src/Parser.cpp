#include "Parser.h"
#include <cctype>
#include <sstream>
#include <iostream>

template<typename T>
std::unique_ptr<T> logError(char const* str) {
  std::cerr << "Error: " << str << std::endl;
  return nullptr;
}

const std::unordered_map<char, int> Parser::BinopPrecedence{
  {'=', 2},
  {'+', 20},
  {'-', 20},
  {'*', 40}
};

const std::unordered_map<char, ArTy> Parser::Tok2ArTy{
  {'i', ArTy::Int},
  {'f', ArTy::Float}
};

int Parser::getTokPrecedence() {
  if (!isascii(curTok)) {
    return -1;
  }

  auto tokPrec = BinopPrecedence.find(curTok);
  if (tokPrec == BinopPrecedence.end()) {
    return -1;
  }
  return tokPrec->second;
}

std::unique_ptr<Expr> Parser::parseNumberExpr() {
  auto result = std::make_unique<Number>(lexer.getNumber());
  getNextToken();
  return std::move(result);
}

std::unique_ptr<Expr> Parser::parseParenExpr() {
  getNextToken(); // skip '('
  auto expr = parseExpression();
  if (!expr) {
    return nullptr;
  }

  if (curTok != ')') {
    return logError<Expr>("expected ')'");
  }
  getNextToken(); // skip ')'
  return expr;
}

std::unique_ptr<Expr> Parser::parseIdentifierExpr() {
  std::string identifier = lexer.getIdentifier();
  
  getNextToken(); // skip identifier

  // Variable
  if (curTok != '(' && curTok != '<') {
    return std::make_unique<Variable>(identifier);
  }

  std::vector<int64_t> tArgs;
  if (curTok == '<') {
    getNextToken(); // skip '<'
    while (true) {
      if (curTok == tok_number) {
        tArgs.push_back(lexer.getNumber());
      } else {
        return logError<Expr>("Expected number");
      }
      getNextToken();
      if (curTok == '>') {
        break;
      }
      if (curTok != ',') {
        return logError<Expr>("Expected '>' or ',' in argument list");
      }
      getNextToken();
    }
    getNextToken(); // skip '>'
  }
  if (curTok != '(') {
    return logError<Expr>("Expected '('");
  }

  // Function call
  getNextToken(); // skip '('
  std::vector<std::unique_ptr<Expr>> args;
  if (curTok != ')') {
    while (true) {
      if (auto arg = parseExpression()) {
        args.push_back(std::move(arg));
      } else {
        return nullptr;
      }

      if (curTok == ')') {
        break;
      }

      if (curTok != ',') {
        return logError<Expr>("Expected ')' or ',' in argument list");
      }
      getNextToken();
    }
  }
  getNextToken(); // skip ')'

  return std::make_unique<Call>(identifier, std::move(args), std::move(tArgs));
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  switch (curTok) {
    default:
      return logError<Expr>("Unknown token when expecting an expression");
    case tok_identifier:
      return parseIdentifierExpr();
    case tok_number:
      return parseNumberExpr();
    case '(':
      return parseParenExpr();
  }
}

std::unique_ptr<Expr> Parser::parseExpression() {
  auto lhs = parsePrimary();
  if (!lhs) {
    return nullptr;
  }

  return parseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<Expr> Parser::parseBinOpRHS(int minPrec, std::unique_ptr<Expr> lhs) {
  while (true) {
    int tokPrec = getTokPrecedence();

    if (tokPrec < minPrec) {
      return lhs;
    }

    auto binOp = curTok;
    getNextToken();
    auto rhs = parsePrimary();
    if (!rhs) {
      return nullptr;
    }

    int nextPrec = getTokPrecedence();
    if (tokPrec < nextPrec) {
      rhs = parseBinOpRHS(tokPrec+1, std::move(rhs));
      if (!rhs) {
        return nullptr;
      }
    }
    lhs = std::make_unique<BinaryOp>(static_cast<char>(binOp), std::move(lhs), std::move(rhs));
  }
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  std::unique_ptr<Stmt> result;
  switch (curTok) {
    default:
      result = parseExpression();
      if (curTok != ';') {
        return logError<Stmt>("Expected ;");
      }
      getNextToken(); // skip ';'
      break;
    case tok_var:
      result = parseDeclaration();
      if (curTok != ';') {
        return logError<Stmt>("Expected ;");
      }
      getNextToken(); // skip ';'
      break;
    case '{':
      result = parseBlock();
      break;
    case tok_for:
      result = parseForStmt();
      break;
  }
  return std::move(result);
}

std::unique_ptr<Block> Parser::parseBlock() {
  getNextToken(); // skip '{'
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (curTok != '}') {
    auto result = parseStatement();
    if (result == nullptr) {
      return nullptr;
    }
    stmts.emplace_back( std::move(result) );
  }
  getNextToken(); // skip '}'
  return std::make_unique<Block>(std::move(stmts));
}


std::unique_ptr<Stmt> Parser::parseForStmt() {
  getNextToken(); // skip for

  if (curTok != tok_identifier) {
    return logError<Stmt>("expected identifier after for");
  }

  std::string idName = lexer.getIdentifier();
  getNextToken(); // skip identifier

  if (curTok != '=') {
    return logError<Stmt>("Expected '=' after for");
  }
  getNextToken(); // skip '='

  auto Start = parseExpression();
  if (!Start) {
    return nullptr;
  }
  if (curTok != ',') {
    return logError<Stmt>("Expected ',' after for start value");
  }
  getNextToken();

  auto End = parseExpression();
  if (!End) {
    return nullptr;
  }

  std::unique_ptr<Expr> Step;
  if (curTok == ',') {
    getNextToken();
    Step = parseExpression();
    if (!Step) {
      return nullptr;
    }
  } else {
    Step = std::make_unique<Number>(1);
  }

  if (curTok != '{') {
    return logError<Stmt>("expected { after for");
  }
  auto Body = parseBlock();
  if (!Body) {
    return nullptr;
  }

  return std::make_unique<For>(idName, std::move(Start),
                                       std::move(End),
                                       std::move(Step),
                                       std::move(Body));
}

std::unique_ptr<VectorTy> Parser::parseVectorTy() {
  getNextToken(); // skip '<'

  auto basic = parseBasicTy();
  if (curTok != ':') {
    return logError<VectorTy>("Expected ':'");
  }
  getNextToken();
  if (curTok != tok_number) {
    return logError<VectorTy>("Expected number");
  }
  auto numElements = lexer.getNumber();
  getNextToken();

  if (curTok != '>') {
    return logError<VectorTy>("Expected '>'");
  }
  getNextToken(); // skip '>'

  return std::make_unique<VectorTy>(*basic, numElements);
}

std::unique_ptr<BasicTy> Parser::parseBasicTy() {
  std::string identifier = lexer.getIdentifier();
  char tyName = identifier[0];
  identifier.erase(0,1);
  if (Tok2ArTy.find(tyName) == Tok2ArTy.end()) {
    return logError<BasicTy>("Unknown type");
  }
  int64_t numBits;
  std::stringstream value(identifier);
  value >> numBits;

  getNextToken();

  bool ptrTy = false;
  if (curTok == '*') {
    ptrTy = true;
    getNextToken();
  }

  return std::make_unique<BasicTy>(Tok2ArTy.at(tyName), numBits, ptrTy);
}

std::unique_ptr<Ty> Parser::parseTy() {
  switch (curTok) {
    default:
      return logError<Ty>("Cannot parse type");
    case tok_identifier:
      return parseBasicTy();
    case '<':
      return parseVectorTy();
  }
}

std::unique_ptr<Stmt> Parser::parseDeclaration() {
  getNextToken(); // skip var

  if (curTok != tok_identifier) {
    return logError<Stmt>("expected identifier after var");
  }

  std::string name = lexer.getIdentifier();
  getNextToken(); // skip identifier

  if (curTok != ':') {
    return logError<Stmt>("expected ':'");
  }
  getNextToken(); // skip ':'

  auto type = parseTy();

  return std::make_unique<Declaration>(std::move(type), name);
}

/*std::unique_ptr<PrototypeAST> Parser::parsePrototype() {
  if (curTok != tok_identifier) {
    return logErrorP("Expected function name in prototype");
  }

  std::string fnName = lexer.getIdentifier();
  getNextToken();

  if (curTok != '(') {
    return logErrorP("Expected '(' in prototype");
  }

  std::vector<std::string> argNames;
  while (getNextToken() == tok_identifier) {
    argNames.push_back(lexer.getIdentifier());
  }
  if (curTok != ')') {
    return logErrorP("Expected ')' in prototype");
  }

  getNextToken(); // skip ')'

  return std::make_unique<PrototypeAST>(fnName, std::move(argNames));
}

std::unique_ptr<FunctionAST> Parser::parseDefinition() {
  getNextToken();
  auto proto = parsePrototype();
  if (!proto) {
    return nullptr;
  }

  if (auto e = parseExpression()) {
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }
  return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::parseExtern() {
  getNextToken();
  return parsePrototype();
}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr() {
  if (auto e = parseExpression()) {
    auto proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }
  return nullptr;
}*/
