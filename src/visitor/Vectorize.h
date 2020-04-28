#ifndef PSPAMM_VISITOR_VECTORIZE_H_
#define PSPAMM_VISITOR_VECTORIZE_H_

#include <vector>
#include <memory>
#include <md/visit.hpp>
#include "AST.h"
#include "VectorizeExpr.h"
#include "CheckTypes.h"
#include "Subs.h"

class Vectorize {
private:
  using return_t = std::unique_ptr<Block>;
  std::string iname;

public:
  Vectorize(std::string const& iname)
    : iname(iname) {}

  return_t operator()(Stmt& stmt) { return nullptr; }

  return_t operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      auto newStmt = md::visit(*this, *stmt);
      if (newStmt) {
        if (block.getStmts().size() == 1) {
          return std::move(newStmt);
        } else {
          stmt = std::move(newStmt);
        }
      }
    }
    return nullptr;
  }

  return_t operator()(For& forLoop) {
    auto newBody = md::visit(*this, forLoop.getBody());
    if (newBody) {
      forLoop.setBody(std::move(newBody));
    }

    if (forLoop.getIname() == iname) {
      auto start = dynamic_cast<Number const*>(&forLoop.getStart());
      auto end = dynamic_cast<Number const*>(&forLoop.getEnd());
      auto step = dynamic_cast<Number const*>(&forLoop.getStep());
      if (!start || !end || !step) {
        std::cerr << "Can only vectorize loops with constant limits" << std::endl;
        return nullptr;
      }
      if (end->getValue() - start->getValue() <= 1) {
        return nullptr;
      }
      if (step->getValue() != 1) {
        std::cerr << "Step must be one" << std::endl;
        return nullptr;
      }
      auto tripCount = end->getValue() - start->getValue();
      auto newBody = md::visit(VectorizeExpr(iname, tripCount), forLoop.getBody());
      std::unique_ptr<Block> newBlock(static_cast<Block*>(newBody.release()));
      md::visit(Subs(iname, *start), *newBlock);
      md::visit(CheckTypes{}, *newBlock);
      return std::move(newBlock);
    }
    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_VECTORIZE_H_

