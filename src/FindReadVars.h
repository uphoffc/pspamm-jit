#ifndef PSPAMM_FINDREADVARS_H_
#define PSPAMM_FINDREADVARS_H_

#include <set>

#include "AST.h"

class FindReadVars {
private:
  std::set<std::string> vars;

public:
  std::set<std::string> const& getVars() const { return vars; }

  void operator()(Stmt& stmt) {}
  void operator()(Variable& var) {
    vars.insert(var.getName());
  }

  void operator()(Load& load) {
    md::visit(*this, load.getSrc());
  }

  void operator()(Store& store) {
    md::visit(*this, store.getSrc());
    md::visit(*this, store.getTarget());
  }

  void operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    } 
  }

  void operator()(AnonymousFunction& fun) {
    md::visit(*this, fun.getBody());
  }

  void operator()(Assign& assign) {
    md::visit(*this, assign.getRHS());
  }

  void operator()(Incr& incr) {
    md::visit(*this, incr.getVar());
  }

  void operator()(Outer& outer) {
    md::visit(*this, outer.getA());
    md::visit(*this, outer.getB());
    md::visit(*this, outer.getC());
  }

  void operator()(For& forLoop) {}
};

#endif // PSPAMM_FINDREADVARS_H_
