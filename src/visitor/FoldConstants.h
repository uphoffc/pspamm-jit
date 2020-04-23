#ifndef PSPAMM_VISITOR_FOLDCONSTANTS_H_
#define PSPAMM_VISITOR_FOLDCONSTANTS_H_

#include <unordered_map>
#include <string>
#include <sstream>

#include "AST.h"
#include "Type.h"

class FoldConstants {
private:
  using return_t = std::unique_ptr<Number>;
  using ValueMap = std::unordered_map<std::string, Number const*>;
  std::vector<ValueMap> values;

  return_t logError(char const* str) {
    std::cerr << "FoldConstants error: " << str << std::endl;
    return nullptr;
  }

  return_t logError(std::string const& str) {
    return logError(str.c_str());
  }

public:
  return_t operator()(Declaration& decl) {
    if (values.back().find(decl.getName()) != values.back().end()) {
      return logError("Variable already declared in local scope");
    }
    values.back()[decl.getName()] = nullptr;
    return nullptr;
  }

  return_t operator()(Variable& var) {
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
      if (it->find(var.getName()) != it->end()) {
        auto& number = it->at(var.getName());
        if (number) {
          return number->cloneNumber();
        }
      }
    }
    return nullptr;
  }

  return_t operator()(Number& number) {
    return number.cloneNumber();
  }

  return_t operator()(BinaryOp& op) {
    auto lhs = md::visit(*this, op.getLHS());
    auto rhs = md::visit(*this, op.getRHS());

    if (rhs && op.getOp() == '=') {
      Variable* lhsE = dynamic_cast<Variable*>(&op.getLHS());
      if (lhsE) {
        if (values.back().find(lhsE->getName()) != values.back().end()) {
          op.setRHS(rhs->cloneNumber());
          values.back()[lhsE->getName()] = static_cast<Number const*>(&op.getRHS());
          return rhs;
        }
      }
      return logError("LHS of assignment must be variable or variable not declared");
    }

    bool mayFold = lhs && rhs;

    if (lhs) {
      op.setLHS(std::move(lhs));
    }
    if (rhs) {
      op.setRHS(std::move(rhs));
    }

    if (!mayFold) {
      return nullptr;
    }

    auto lNum = static_cast<Number&>(op.getLHS()).getValue();
    auto rNum = static_cast<Number&>(op.getRHS()).getValue();

    switch (op.getOp()) {
      case '+':
        return std::make_unique<Number>(lNum + rNum);
      case '-':
        return std::make_unique<Number>(lNum - rNum);
      case '*':
        return std::make_unique<Number>(lNum * rNum);
      default:
        return logError("Unknown operator");
    }

    return nullptr;
  }

  return_t operator()(Block& block) {
    values.push_back(ValueMap{});
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

#endif // PSPAMM_VISITOR_FOLDCONSTANTS_H_

