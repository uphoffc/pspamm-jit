#ifndef PSPAMM_AST_H_
#define PSPAMM_AST_H_

#include <memory>
#include <utility>
#include <string>
#include <vector>

#include <md/type.hpp>

#include "Type.h"

using StmtBase = md::type<class Declaration,
                          class Variable,
                          class Number,
                          class BinaryOp,
                          class Call,
                          class Block,
                          class For,
                          class Fn>;
class Stmt : public StmtBase {
public:
  virtual std::unique_ptr<Stmt> clone() const = 0;
};

class Declaration : public md::with_type<Declaration,Stmt> {
private:
  std::unique_ptr<Ty> type;
  std::string name;

public:
  Declaration(std::unique_ptr<Ty> type, std::string const& name)
    : type(std::move(type)), name(name) {
  }
  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<Declaration>(type->clone(), name);
  }

  Ty const& getTy() const { return *type; }
  std::string const& getName() const { return name; }
};

class Expr : public Stmt {
protected:
  Ty const* type;

public:
  Expr() : type(nullptr) {}
  Expr(Ty const* type) : type(type) {}
  virtual std::unique_ptr<Expr> cloneExpr() const = 0;
  std::unique_ptr<Stmt> clone() const override {
    return cloneExpr();
  }

  virtual Ty const* getTy() const { return type; }
  virtual void setTy(Ty const* ty) { type = ty; }

  virtual bool isZero() const { return false; }
  virtual bool isIdentity() const { return false; }
};

class Variable : public md::with_type<Variable,Expr> {
private:
  std::string name;

public:
  Variable(std::string const& name)
    : name(name) {
  }
  std::unique_ptr<Expr> cloneExpr() const override {
    return std::make_unique<Variable>(*this);
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

  std::unique_ptr<Number> cloneNumber() const {
    return std::make_unique<Number>(value);
  }
  std::unique_ptr<Expr> cloneExpr() const override {
    return cloneNumber();
  }

  int64_t getValue() const { return value; }
  virtual Ty const* getTy() const { return myTy.get(); }

  bool isZero() const override { return value == 0; }
  bool isIdentity() const override { return value == 1; }
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
  std::unique_ptr<Expr> cloneExpr() const override {
    return std::make_unique<BinaryOp>(op,
                                      lhs->cloneExpr(),
                                      rhs->cloneExpr());
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
  std::vector<int64_t> tArgs;

public:
  Call(std::string_view callee,
       std::vector<std::unique_ptr<Expr>> args,
       std::vector<int64_t> tArgs = std::vector<int64_t>{})
    : callee(callee), args(std::move(args)), tArgs(std::move(tArgs)) {}

  std::unique_ptr<Call> cloneCall() const {
    std::vector<std::unique_ptr<Expr>> argsClone;
    for (auto& arg : args) {
      argsClone.emplace_back(arg->cloneExpr());
    }
    return std::make_unique<Call>(callee, std::move(argsClone), tArgs);
  }

  std::unique_ptr<Expr> cloneExpr() const override {
    return cloneCall();
  }

  auto& getArgs() { return args; }
  auto& getTArgs() { return tArgs; }

  std::string const& getCallee() const { return callee; }
};

class Block : public md::with_type<Block,Stmt> {
private:
  std::vector<std::unique_ptr<Stmt>> stmts;

public:
  Block(std::vector<std::unique_ptr<Stmt>> stmts)
    : stmts(std::move(stmts)) {
  }
  std::unique_ptr<Block> cloneBlock() const {
    std::vector<std::unique_ptr<Stmt>> stmtsClone;
    for (auto& stmt : stmts) {
      stmtsClone.emplace_back(stmt->clone());
    }
    return std::make_unique<Block>(std::move(stmtsClone));
  }
  std::unique_ptr<Stmt> clone() const override {
    return cloneBlock();
  }

  auto& getStmts() { return stmts; }
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
  std::unique_ptr<For> cloneFor() const {
    std::unique_ptr<Block> clonedBody;
    if (body) {
      clonedBody = body->cloneBlock();
    }
    return std::make_unique<For>(iname,
                                 start->cloneExpr(),
                                 end->cloneExpr(),
                                 step->cloneExpr(),
                                 std::move(clonedBody));
  }
  std::unique_ptr<Stmt> clone() const override {
    return cloneFor();
  }

  std::string const& getIname() const { return iname; }
  Expr& getStart() { return *start; }
  Expr& getEnd() { return *end; }
  Expr& getStep() { return *step; }
  Block& getBody() { return *body; }

  std::unique_ptr<Block> transBody() { return std::move(body); }

  void setStart(std::unique_ptr<Expr> expr) { start = std::move(expr); }
  void setEnd(std::unique_ptr<Expr> expr) { end = std::move(expr); }
  void setStep(std::unique_ptr<Expr> expr) { step = std::move(expr); }
  void setBody(std::unique_ptr<Block> bdy) { body = std::move(bdy); }
};

class Fn : public md::with_type<Fn,Stmt> {
private:
  std::vector<std::string> args;
  std::unique_ptr<Stmt> body;

public:
  Fn(std::vector<std::string> args, std::unique_ptr<Stmt> body)
    : args(std::move(args)), body(std::move(body)) {
  }
  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<Fn>(args, body->clone());
  }

  auto& getArgs() const { return args; }
  Stmt& getBody() { return *body; }
};

#endif // PSPAMM_AST_H_
