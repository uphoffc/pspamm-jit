#ifndef PSPAMM_VISITOR_MOVELOOP_H_
#define PSPAMM_VISITOR_MOVELOOP_H_

#include <md/visit.hpp>
#include "AST.h"
#include "FindAccessedInBlock.h"

class MoveLoop {
private:
  using return_t = std::unique_ptr<Stmt>;
  std::string iname;

public:
  MoveLoop(std::string const& iname)
    : iname(iname) {}

  return_t operator()(Stmt& stmt) { return nullptr; }

  return_t operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      auto nw = md::visit(*this, *stmt);
      if (nw) {
        stmt = std::move(nw);
        md::visit(*this, *stmt);
      }
    }
    return nullptr;
  }

  return_t operator()(For& forLoop) {
    if (forLoop.getIname() == iname) {
      FindAccessedInBlock accessed;
      md::visit(accessed, forLoop.getBody());
      auto& av = accessed.getVars();
      if (av.find(iname) == av.end()) {
        if (forLoop.getBody().getStmts().size() == 1) {
          For* child = dynamic_cast<For*>(forLoop.getBody().getStmts()[0].get());
          if (child) {
            std::unique_ptr<Block> childBody = child->transBody();
            std::unique_ptr<Block> forLoopBody = forLoop.transBody();
            std::unique_ptr<For> childLoop(static_cast<For*>(forLoopBody->getStmts()[0].release()));
            std::unique_ptr<For> newForLoop = forLoop.cloneFor();
            newForLoop->setBody(std::move(childBody));
            forLoopBody->getStmts()[0] = std::move(newForLoop);
            childLoop->setBody(std::move(forLoopBody));
            return childLoop;
          }
        }
      }
    }

    md::visit(*this, forLoop.getBody());
    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_MOVELOOP_H_

