#ifndef PSPAMM_VISITOR_FINDACCESSEDINBLOCK_H_
#define PSPAMM_VISITOR_FINDACCESSEDINBLOCK_H_

#include <set>

#include "AST.h"

class FindAccessedInBlock {
private:
  std::set<std::string> vars;

public:
  std::set<std::string> const& getVars() const { return vars; }

  void operator()(Variable& var) {
    vars.insert(var.getName());
  }

  void operator()(BinaryOp& binop) {
    md::visit(*this, binop.getLHS());
    md::visit(*this, binop.getRHS());
  }

  void operator()(Block& block) {
    for (auto& arg : block.getStmts()) {
      md::visit(*this, *arg);
    }
  }

  void operator()(Call& call) {
    for (auto& arg : call.getArgs()) {
      md::visit(*this, *arg);
    }
  }

  void operator()(For& forLoop) {
    vars.insert(forLoop.getIname());
    md::visit(*this, forLoop.getStart());
    md::visit(*this, forLoop.getEnd());
    md::visit(*this, forLoop.getStep());
  }

  void operator()(Stmt&) {}
};

#endif // PSPAMM_VISITOR_FINDACCESSEDINBLOCK_H_

