#ifndef PSPAMM_PRETTYPRINTER_H_
#define PSPAMM_PRETTYPRINTER_H_

#include <ostream>
#include <string>

#include "AST.h"

class PrettyPrinter {
private:
  int level = 0;
  std::ostream& out;

  std::string indent() const {
    return std::string(2*level, ' ');
  }

public:
  PrettyPrinter(std::ostream& out)
    : out(out) {}

  void operator()(Stmt& stmt) {}
  void operator()(Variable& var) {
    out << var.getName();
  }

  void operator()(Load& load) {
    out << "Load(";
    md::visit(*this, load.getSrc()),
    out << ", "
        << load.getMb() << ", "
        << load.getNb() << ")";
  }

  void operator()(Store& store) {
    out << indent() << "Store(";
    md::visit(*this, store.getSrc());
    out << ", ";
    md::visit(*this, store.getTarget());
    out << ")" << std::endl;
  }

  void operator()(Block& block) {
    ++level;
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    } 
    --level;
  }

  void operator()(AnonymousFunction& fun) {
    out << "function ( ";
    for (auto& arg : fun.getArgs()) {
      out << arg << " ";
    }
    out << ")" << std::endl;
    ++level;
    md::visit(*this, fun.getBody());
    --level;
  }

  void operator()(Assign& assign) {
    out << indent();
    md::visit(*this, assign.getLHS());
    out << " := ";
    md::visit(*this, assign.getRHS());
    out << std::endl;
  }

  void operator()(Incr& incr) {
    md::visit(*this, incr.getVar());
    out << " + " << incr.getIncr();
  }

  void operator()(Outer& outer) {
    out << "outer(";
    md::visit(*this, outer.getA());
    out << ", ";
    md::visit(*this, outer.getB());
    out << ", ";
    md::visit(*this, outer.getC());
    out << ")";
  }

  void operator()(For& forLoop) {
    out << indent() << "count from "
        << forLoop.getStart()
        << " up to "
        << forLoop.getStop()-1
        << " with step "
        << forLoop.getStep() << std::endl;
    ++level;
    md::visit(*this, forLoop.getBody());
    --level;
  }
};

#endif // PSPAMM_PRETTYPRINTER_H_
