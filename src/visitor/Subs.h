#ifndef PSPAMM_VISITOR_SUBS_H_
#define PSPAMM_VISITOR_SUBS_H_

#include <set>
#include <string>
#include <memory>

#include "AST.h"

class Subs {
private:
  using return_t = std::unique_ptr<Expr>;
  using DeclSet = std::set<std::string>;
  std::vector<DeclSet> values;

  std::string varName;
  std::unique_ptr<Expr> by;

public:
  Subs(std::string const& varName, std::unique_ptr<Expr> by)
    : varName(varName), by(std::move(by)) {}

  return_t operator()(Stmt& stmt) {
    return nullptr;
  }

  return_t operator()(Declaration& decl) {
    values.back().insert(decl.getName());
    return nullptr;
  }

  return_t operator()(Variable& var) {
    if (var.getName() != varName) {
      return nullptr;
    }
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
      if (it->find(var.getName()) != it->end()) {
        return nullptr;
      }
    }
    return by->cloneExpr();
  }

  return_t operator()(BinaryOp& op) {
    auto rhs = md::visit(*this, op.getRHS());
    if (rhs) {
      op.setRHS(std::move(rhs));
    }

    if (op.getOp() != '=') {
      auto lhs = md::visit(*this, op.getLHS());
      if (lhs) {
        op.setLHS(std::move(lhs));
      }
    }

    return nullptr;
  }

  return_t operator()(Block& block) {
    values.push_back(DeclSet{});
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    }
    values.pop_back();
    return nullptr;
  }

  return_t operator()(For& forLoop) {
    auto fold = [&](Expr& (For::*get)(), void (For::*set)(std::unique_ptr<Expr>)) {
      auto expr = md::visit(*this, (forLoop.*get)());
      if (expr) {
        (forLoop.*set)(std::move(expr));
      }
    };
    fold(&For::getStart, &For::setStart);
    fold(&For::getEnd, &For::setEnd);
    fold(&For::getStep, &For::setStep);
    md::visit(*this, forLoop.getBody());
    return nullptr;
  }

  return_t operator()(Call& call) {
    for (auto& arg : call.getArgs()) {
      auto expr = md::visit(*this, *arg);
      if (expr) {
        arg = std::move(expr);
      }
    }
    return nullptr;
  }

  return_t operator()(Fn& fn) {
    // TODO
    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_SUBS_H_

