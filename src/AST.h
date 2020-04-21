#ifndef PSPAMM_AST_H_
#define PSPAMM_AST_H_

#include <memory>
#include <utility>
#include <string>
#include <vector>

#include <md/type.hpp>

using Stmt = md::type<class Expr,
                      class Variable,
                      class Ptr,
                      class Matrix,
                      class Assign,
                      class Block,
                      class AnonymousFunction,
                      class For,
                      class Load,
                      class Store,
                      class Outer,
                      class Incr>;

class Expr : public md::with_type<Expr,Stmt> {};

class Variable : public md::with_type<Variable,Expr> {
private:
  std::string name;

public:
  Variable(std::string const& name)
    : name(name) {
  }

  std::string const& getName() const { return name; }
};

class Ptr : public md::with_type<Ptr,Variable> {
public:
  using md::with_type<Ptr,Variable>::with_type;
};

class Matrix : public md::with_type<Matrix,Variable> {
private:
  int Mb, Nb;

public:
  Matrix(std::string const& name, int Mb, int Nb)
    : md::with_type<Matrix,Variable>(name), Mb(Mb), Nb(Nb) {
  }

  int getMb() const { return Mb; }
  int getNb() const { return Nb; }
};

class Assign : public md::with_type<Assign,Stmt> {
private:
  std::unique_ptr<Variable> lhs;
  std::unique_ptr<Expr> rhs;

public:
  Assign(std::unique_ptr<Variable> lhs, std::unique_ptr<Expr> rhs)
    : lhs(std::move(lhs)), rhs(std::move(rhs)) {
  }

  Variable& getLHS() { return *lhs; }
  Expr& getRHS() { return *rhs; }
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

class AnonymousFunction : public md::with_type<AnonymousFunction,Stmt> {
private:
  std::vector<std::string> args;
  std::unique_ptr<Stmt> body;

public:
  AnonymousFunction(std::vector<std::string> args, std::unique_ptr<Stmt> body)
    : args(std::move(args)), body(std::move(body)) {
  }

  auto& getArgs() const { return args; }
  Stmt& getBody() { return *body; }
};

class For : public md::with_type<For,Stmt> {
private:
  int start, stop, step;
  std::unique_ptr<Stmt> body;

public:
  For(int start, int stop, int step, std::unique_ptr<Block> body)
    : start(start), stop(stop), step(step), body(std::move(body)) {
  }

  int getStart() const { return start; }
  int getStop() const { return stop; }
  int getStep() const { return step; }
  Stmt& getBody() { return *body; }
};

class Load : public md::with_type<Load,Expr> {
private:
  std::unique_ptr<Ptr> src;
  int Mb, Nb, ld;

public:
  Load(std::unique_ptr<Ptr> src, int Mb, int Nb, int ld)
    : src(std::move(src)), Mb(Mb), Nb(Nb), ld(ld) {
  }

  Variable& getSrc() { return *src; }
  int getMb() const { return Mb; }
  int getNb() const { return Nb; }
  int getLd() const { return ld; }
};

class Store : public md::with_type<Store,Expr> {
private:
  std::unique_ptr<Matrix> src;
  std::unique_ptr<Ptr> target;
  int ld;

public:
  Store(std::unique_ptr<Matrix> src, std::unique_ptr<Ptr> target, int ld)
    : src(std::move(src)), target(std::move(target)), ld(ld) {
  }

  Matrix& getSrc() { return *src; }
  Ptr& getTarget() { return *target; }
  int getLd() const { return ld; }
};

class Outer : public md::with_type<Outer,Expr> {
private:
  std::unique_ptr<Matrix> a;
  std::unique_ptr<Matrix> b;
  std::unique_ptr<Matrix> c;

public:
  Outer(std::unique_ptr<Matrix> a, std::unique_ptr<Matrix> b, std::unique_ptr<Matrix> c) 
    : a(std::move(a)), b(std::move(b)), c(std::move(c)) {
  }

  Matrix& getA() { return *a; }
  Matrix& getB() { return *b; }
  Matrix& getC() { return *c; }
};

class Incr : public md::with_type<Incr,Expr> {
private:
  std::unique_ptr<Variable> var;
  int increment; 

public:
  Incr(std::unique_ptr<Variable> var, int increment) 
    : var(std::move(var)), increment(increment) {
  }

  Variable& getVar() { return *var; }
  int getIncr() const { return increment; }
};

#endif // PSPAMM_AST_H_
