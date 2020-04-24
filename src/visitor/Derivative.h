#ifndef PSPAMM_VISITOR_DERIVATIVE_H_
#define PSPAMM_VISITOR_DERIVATIVE_H_

#include <memory>
#include <md/visit.hpp>
#include "AST.h"
#include "SimplifyExpr.h"

class Derivative {
private:
  using return_t = std::unique_ptr<Expr>;
  std::string x;

public:
  Derivative(std::string const& x) : x(x) {}

  return_t operator()(Stmt&) {
    return nullptr;
  }

  return_t operator()(Variable& var) {
    if (var.getName() == x) {
      return std::make_unique<Number>(1);
    }
    return std::make_unique<Number>(0);
  }

  return_t operator()(Number& number) {
    return std::make_unique<Number>(0);
  }

  return_t operator()(BinaryOp& op) {
    if (op.getOp() == '=') {
      return nullptr;
    }

    auto lhs = md::visit(*this, op.getLHS());
    auto rhs = md::visit(*this, op.getRHS());

    if (!lhs || !rhs) {
      return nullptr;
    }

    switch (op.getOp()) {
      case '+':
        return std::make_unique<BinaryOp>('+', std::move(lhs), std::move(rhs));
      case '-':
        return std::make_unique<BinaryOp>('-', std::move(lhs), std::move(rhs));
      case '*': {
        bool isZero = lhs->isZero() || rhs->isZero();
        auto newOp = std::make_unique<BinaryOp>('+',
            std::make_unique<BinaryOp>('*', op.getLHS().cloneExpr(), std::move(rhs)),
            std::make_unique<BinaryOp>('*', std::move(lhs), op.getRHS().cloneExpr())
        );
        if (isZero) {
          return md::visit(SimplifyExpr{}, *newOp);
        }
        return std::move(newOp);
      }
      default:
        return nullptr;
    }

    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_DERIVATIVE_H_

