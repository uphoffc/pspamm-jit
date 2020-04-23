#ifndef PSPAMM_VISITOR_FINDMODIFIED_H_
#define PSPAMM_VISITOR_FINDMODIFIED_H_

#include <set>

#include "AST.h"
#include "Traverse.h"

class FindModified : public Traverse<FindModified> {
private:
  std::set<std::string> vars;

public:
  using Traverse<FindModified>::operator();

  std::set<std::string> const& getVars() const { return vars; }

  void operator()(BinaryOp& binop) {
    md::visit(*this, binop.getLHS());
    md::visit(*this, binop.getRHS());
    if (binop.getOp() == '=') {
      Variable* lhs = dynamic_cast<Variable*>(&binop.getLHS());
      vars.insert(lhs->getName());
    }
  }

  void operator()(Stmt&) {}
};

#endif // PSPAMM_VISITOR_FINDMODIFIED_H_

