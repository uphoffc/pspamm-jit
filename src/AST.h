#ifndef PSPAMM_AST_H_
#define PSPAMM_AST_H_

#include <memory>
#include <utility>
#include <string>
#include <vector>

#include <md/type.hpp>

#include "Type.h"

using Stmt = md::type<class Declaration,
                      class Expr,
                      class Variable,
                      class Number,
                      class BinaryOp,
                      class Call,
                      class Block,
                      class For,
                      class Fn>;

class Declaration : public md::with_type<Declaration,Stmt> {
private:
  std::unique_ptr<Ty> type;
  std::string name;

public:
  Declaration(std::unique_ptr<Ty> type, std::string const& name)
    : type(std::move(type)), name(name) {
  }

  Ty const& getTy() const { return *type; }
  std::string const& getName() const { return name; }
};

class Expr : public md::with_type<Expr,Stmt> {
protected:
  Ty const* type;

public:
  Expr() : type(nullptr) {}
  Expr(Ty const* type) : type(type) {}

  virtual Ty const* getTy() const { return type; }
  virtual void setTy(Ty const* ty) { type = ty; }
};

class Variable : public md::with_type<Variable,Expr> {
private:
  std::string name;

public:
  Variable(std::string const& name)
    : name(name) {
  }

  std::string const& getName() const { return name; }
};

class Number : public md::with_type<Number,Expr> {
private:
  std::unique_ptr<Ty> myTy;
  using Expr::setTy;

  int64_t value;

public:
  Number(int64_t value)
    : myTy(std::make_unique<BasicTy>(ArTy::Int,64,false)), value(value) {}

  int64_t getValue() const { return value; }
  virtual Ty const* getTy() const { return myTy.get(); }
};

class BinaryOp : public md::with_type<BinaryOp,Expr> {
private:
  char op;
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;

public:
  BinaryOp(char op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {
  }

  char getOp() const { return op; }
  Expr& getLHS() { return *lhs; }
  Expr& getRHS() { return *rhs; }

  std::unique_ptr<Expr> transLHS() { return std::move(lhs); }
  std::unique_ptr<Expr> transRHS() { return std::move(rhs); }

  void setLHS(std::unique_ptr<Expr> newLHS) { lhs = std::move(newLHS); }
  void setRHS(std::unique_ptr<Expr> newRHS) { rhs = std::move(newRHS); }
};

class Call : public md::with_type<Call,Expr> {
private:
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;

public:
  Call(std::string const& callee, std::vector<std::unique_ptr<Expr>> args)
    : callee(callee), args(std::move(args)) {}

  auto const& getArgs() const { return args; }

  std::string const& getCallee() const { return callee; }
};

class Block : public md::with_type<Block,Stmt> {
private:
  std::vector<std::unique_ptr<Stmt>> stmts;

public:
  Block(std::vector<std::unique_ptr<Stmt>> stmts)
    : stmts(std::move(stmts)) {
  }

  auto& getStmts() { return stmts; }
};

class Fn : public md::with_type<Fn,Stmt> {
private:
  std::vector<std::string> args;
  std::unique_ptr<Stmt> body;

public:
  Fn(std::vector<std::string> args, std::unique_ptr<Stmt> body)
    : args(std::move(args)), body(std::move(body)) {
  }

  auto& getArgs() const { return args; }
  Stmt& getBody() { return *body; }
};

class For : public md::with_type<For,Stmt> {
private:
  std::string iname;
  std::unique_ptr<Expr> start, end, step;
  std::unique_ptr<Block> body;

public:
  For(std::string const& iname,
      std::unique_ptr<Expr> start,
      std::unique_ptr<Expr> end,
      std::unique_ptr<Expr> step,
      std::unique_ptr<Block> body)
    : iname(iname),
      start(std::move(start)),
      end(std::move(end)),
      step(std::move(step)),
      body(std::move(body)) {
  }

  std::string const& getIname() const { return iname; }
  Expr& getStart() { return *start; }
  Expr& getEnd() { return *end; }
  Expr& getStep() { return *step; }
  Block& getBody() { return *body; }
};

#endif // PSPAMM_AST_H_
