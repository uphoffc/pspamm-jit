#ifndef PSPAMM_VISITOR_SIMPLIFYEXPR_H_
#define PSPAMM_VISITOR_SIMPLIFYEXPR_H_

#include <memory>
#include <md/visit.hpp>
#include "AST.h"

class SimplifyExpr {
private:
  using return_t = std::unique_ptr<Expr>;

public:
  return_t operator()(Stmt&) {
    return nullptr;
  }

  return_t operator()(Expr& expr) {
    return expr.cloneExpr();
  }

  return_t operator()(BinaryOp& op) {
    auto rhs = md::visit(*this, op.getRHS());
    if (rhs && op.getOp() == '=') {
      return std::make_unique<BinaryOp>('=', op.getLHS().cloneExpr(), std::move(rhs));
    }

    auto lhs = md::visit(*this, op.getLHS());

    if (!lhs || !rhs) {
      return nullptr;
    }

    switch (op.getOp()) {
      case '+':
        if (lhs->isZero()) {
          return std::move(rhs);
        }
        if (rhs->isZero()) {
          return std::move(lhs);
        }
        return std::make_unique<BinaryOp>('+', std::move(lhs), std::move(rhs));
      case '-':
        if (rhs->isZero()) {
          return std::move(lhs);
        }
        return std::make_unique<BinaryOp>('-', std::move(lhs), std::move(rhs));
      case '*': {
        if (lhs->isZero() || rhs->isZero()) {
          return std::make_unique<Number>(0);
        }
        if (lhs->isIdentity()) {
          return std::move(rhs);
        }
        if (rhs->isIdentity()) {
          return std::move(lhs);
        }
        return std::make_unique<BinaryOp>('*', std::move(lhs), std::move(rhs));
      }
      default:
        return nullptr;
    }

    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_SIMPLIFYEXPR_H_

