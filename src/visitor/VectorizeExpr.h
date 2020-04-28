#ifndef PSPAMM_VISITOR_VECTORIZEEXPR_H_
#define PSPAMM_VISITOR_VECTORIZEEXPR_H_

#include <memory>
#include <set>
#include <string>
#include <md/type.hpp>

#include "AST.h"
#include "Type.h"
#include "Derivative.h"
#include "SimplifyExpr.h"

class VectorizeExpr {
private:
  using return_t = std::unique_ptr<Stmt>;
  std::string iname;
  int64_t newVL;

  std::set<std::string> declUpdate;

  return_t logError(char const* str) {
    std::cerr << "VectorizeExpr error: " << str << std::endl;
    return nullptr;
  }

public:
  VectorizeExpr(std::string const& iname, int64_t vl)
    : iname(iname), newVL(vl) {}

  return_t operator()(Stmt& expr) {
    return logError("Can only vectorize inner-most loop");
  }

  return_t operator()(Expr& expr) {
    return nullptr;
  }

  return_t operator()(Declaration& decl) {
    if (declUpdate.find(decl.getName()) != declUpdate.end()) {
      declUpdate.erase(decl.getName());
      auto ty = dynamic_cast<BasicTy const*>(&decl.getTy());
      if (!ty) {
        return logError("Cannot update declaration to vector type");
      }
      return std::make_unique<Declaration>(
          std::make_unique<VectorTy>(*ty, newVL),
          decl.getName());
    }
    return nullptr;
  }

  return_t operator()(BinaryOp& op) {
    auto rhs = md::visit(*this, op.getRHS());
    if (rhs) {
      op.setRHS(std::unique_ptr<Expr>(static_cast<Expr*>(rhs.release())));
      if (op.getOp() == '=') {
        declUpdate.insert(static_cast<Variable&>(op.getLHS()).getName());
      }
    }
    auto lhs = md::visit(*this, op.getLHS());
    if (lhs) {
      op.setLHS(std::unique_ptr<Expr>(static_cast<Expr*>(lhs.release())));
    }
    return nullptr;
  }

  return_t operator()(Block& block) {
    std::vector<std::unique_ptr<Stmt>> newStmts;
    for (auto stmt = block.getStmts().rbegin(); stmt != block.getStmts().rend(); ++stmt) {
      auto newStmt = md::visit(*this, **stmt);
      if (newStmt) {
        newStmts.emplace(newStmts.begin(), std::move(newStmt));
      } else {
        newStmts.emplace(newStmts.begin(), std::move(*stmt));
      }
    }
    return std::make_unique<Block>(std::move(newStmts));
  }

  return_t operator()(Call& call) {
    if (call.getCallee() == "load") {
      auto vl = call.getTArgs()[0];
      if (vl != 1) {
        return logError("Cannot vectorize load with vector length not equal to 1");
      }
      auto der = md::visit(Derivative(iname), *call.getArgs()[1]);
      if (der) {
        auto derSimple = md::visit(SimplifyExpr{}, *der);
        auto number = dynamic_cast<Number*>(derSimple.get());
        if (number) {
          if (number->getValue() == 1) {
            auto c = call.cloneCall();
            c->getTArgs()[0] = newVL;
            return std::move(c);
          }
          if (number->getValue() == 0) {
            return nullptr;
          }
        }
        return logError("Cannot vectorize call: Address derivative must be 1 or 0");
      }
      return logError("Cannot compute derivative");
    }
    if (call.getCallee() == "store") {
      return nullptr;
    }
    return logError("Cannot vectorize call: Unknown function");
  }
};

#endif // PSPAMM_VISITOR_VECTORIZEEXPR_H_

