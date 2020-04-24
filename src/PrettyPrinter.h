#ifndef PSPAMM_PRETTYPRINTER_H_
#define PSPAMM_PRETTYPRINTER_H_

#include <ostream>
#include <iostream>
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
  PrettyPrinter(std::ostream& out = std::cout)
    : out(out) {}

  void operator()(Stmt&) {}

  void operator()(Ty const& ty) {}
  void operator()(BasicTy const& ty) {
    out << ty.getTyName() << ty.getNumBits() << (ty.isPtrTy() ? "*" : "");
  }
  void operator()(VectorTy const& ty) {
    out << "<";
    auto basic = ty.getBasicTy();
    md::visit(*this, basic);
    out << ":" << ty.getNumElements();
    out << ">";
  }

  void operator()(Variable& var) {
    out << var.getName();
  }

  void operator()(Number& number) {
    out << number.getValue();
  }

  void operator()(BinaryOp& op) {
    bool printParen = op.getOp() == '+' || op.getOp() == '-';
    if (printParen) {
      out << "(";
    }
    md::visit(*this, op.getLHS());
    out << " " << op.getOp() << " ";
    md::visit(*this, op.getRHS());
    if (printParen) {
      out << ")";
    }
  }

  void operator()(Block& block) {
    out << "{" << std::endl;
    ++level;
    for (auto& stmt : block.getStmts()) {
      out << indent();
      md::visit(*this, *stmt);
      out << std::endl;
    }
    --level;
    out << indent() << "}";
  }

  void operator()(For& forLoop) {
    out << "for " << forLoop.getIname() << "=";
    md::visit(*this, forLoop.getStart());
    out << ", ";
    md::visit(*this, forLoop.getEnd());
    out << ", ";
    md::visit(*this, forLoop.getStep());
    out << " ";
    md::visit(*this, forLoop.getBody());
  }

  void operator()(Declaration& decl) {
    out << "var " << decl.getName() << ": ";
    md::visit(*this, decl.getTy());
  }

  void operator()(Call& call) {
    out << call.getCallee() << "(";
    for (auto& arg : call.getArgs()) {
      md::visit(*this, *arg);
      out << ",";
    }
    out << ")";
  }
};

#endif // PSPAMM_PRETTYPRINTER_H_
