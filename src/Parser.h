#ifndef PSPAMM_PARSER_H_
#define PSPAMM_PARSER_H_

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Lexer.h"
#include "AST.h"
#include "Type.h"

class Parser {
//private:
public:
  int64_t curTok;
  Lexer& lexer;

  int getTokPrecedence();

public:
  static const std::unordered_map<char, int> BinopPrecedence;
  static const std::unordered_map<char, ArTy> Tok2ArTy;

  Parser(Lexer& lexer)
    : lexer(lexer) {}

  int64_t getNextToken() {
    return curTok = lexer.getToken();
  }

  std::unique_ptr<Expr> parseNumberExpr();
  std::unique_ptr<Expr> parseParenExpr();
  std::unique_ptr<Expr> parseIdentifierExpr();
  std::unique_ptr<Expr> parsePrimary();
  std::unique_ptr<Expr> parseExpression();
  std::unique_ptr<Expr> parseBinOpRHS(int minPrec, std::unique_ptr<Expr> lhs);
  std::unique_ptr<Stmt> parseStatement();
  std::unique_ptr<Block> parseBlock();
  std::unique_ptr<Stmt> parseForStmt();
  std::unique_ptr<VectorTy> parseVectorTy();
  std::unique_ptr<BasicTy> parseBasicTy();
  std::unique_ptr<Ty> parseTy();
  std::unique_ptr<Stmt> parseDeclaration();
  /*std::unique_ptr<PrototypeAST> parsePrototype();
  std::unique_ptr<FunctionAST> parseDefinition();
  std::unique_ptr<PrototypeAST> parseExtern();
  std::unique_ptr<FunctionAST> parseTopLevelExpr();*/
};

#endif // PSPAMM_PARSER_H_
