#ifndef PSPAMM_VISITOR_VECTORIZE_H_
#define PSPAMM_VISITOR_VECTORIZE_H_

#include <vector>
#include <memory>
#include <md/visit.hpp>
#include "AST.h"
#include "Derivative.h"
#include "SimplifyExpr.h"
#include "PrettyPrinter.h"

class Vectorize {
private:
  using return_t = std::unique_ptr<Stmt>;
  std::string iname;

public:
  Vectorize(std::string const& iname)
    : iname(iname) {}

  return_t operator()(Stmt& stmt) { return nullptr; }

  return_t operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    }
    return nullptr;
  }

  return_t operator()(For& forLoop) {
    if (forLoop.getIname() == iname) {
      std::vector<std::unique_ptr<Stmt>> stmts;
      for (auto& stmt : forLoop.getBody().getStmts()) {
        auto der = md::visit(Derivative(iname), *stmt);
        if (der) {
          auto derSimple = md::visit(SimplifyExpr{}, *der);
          md::visit(PrettyPrinter{}, *stmt);
          std::cout << std::endl;
          md::visit(PrettyPrinter{}, *derSimple);
          std::cout << std::endl << std::endl;
        }
      }

      //forLoop.setBody(std::make_unique<Block>(std::move(stmts)));
      return nullptr;
    }

    md::visit(*this, forLoop.getBody());
    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_VECTORIZE_H_

