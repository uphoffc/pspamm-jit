#ifndef PSPAMM_VISITOR_TRAVERSE_H_
#define PSPAMM_VISITOR_TRAVERSE_H_

#include "AST.h"

template<typename Derived, typename Ret = void>
class Traverse {
public:
  using return_t = Ret;

  return_t operator()(Stmt& stmt) { return return_t(); }

  return_t operator()(BinaryOp& op) {
    md::visit(*static_cast<Derived*>(this), op.getLHS());
    md::visit(*static_cast<Derived*>(this), op.getRHS());
    return return_t();
  }

  return_t operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      md::visit(*static_cast<Derived*>(this), *stmt);
    }
    return return_t();
  }

  return_t operator()(For& forLoop) {
    md::visit(*static_cast<Derived*>(this), forLoop.getStart());
    md::visit(*static_cast<Derived*>(this), forLoop.getEnd());
    md::visit(*static_cast<Derived*>(this), forLoop.getStep());
    md::visit(*static_cast<Derived*>(this), forLoop.getBody());
    return return_t();
  }

  return_t operator()(Call& call) {
    for (auto& arg : call.getArgs()) {
      md::visit(*static_cast<Derived*>(this), *arg);
    }
    return return_t();
  }
};

#endif // PSPAMM_VISITOR_TRAVERSE_H_

