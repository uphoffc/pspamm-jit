#ifndef PSPAMM_VISITOR_UNROLL_H_
#define PSPAMM_VISITOR_UNROLL_H_

#include <string>
#include <set>

#include "AST.h"

class Unroll {
private:
  std::set<std::string> inames;

public:
  Unroll(std::string const& iname) : inames{iname} {}
  Unroll(std::initializer_list<std::string> inames) : inames(inames) {}

  std::unique_ptr<Block> operator()(Stmt& stmt) { return nullptr; }

  std::unique_ptr<Block> operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      auto newStmt = md::visit(*this, *stmt);
      if (newStmt) {
        stmt = std::move(newStmt);
      }
    }
    return nullptr;
  }

  std::unique_ptr<Block> operator()(For& forLoop) {
    md::visit(*this, forLoop.getBody());

    if (inames.find(forLoop.getIname()) != inames.end()) {
      auto start = dynamic_cast<Number const*>(&forLoop.getStart());
      auto end = dynamic_cast<Number const*>(&forLoop.getEnd());
      auto step = dynamic_cast<Number const*>(&forLoop.getStep());
      if (!start || !end || !step) {
        std::cout << "Can only unroll loops with constant limits" << std::endl;
        return nullptr;
      }
      std::vector<std::unique_ptr<Stmt>> stmts;
      stmts.emplace_back(std::make_unique<Declaration>(
        start->getTy()->clone(),
        forLoop.getIname()
      ));
      for (auto& arg : forLoop.getBody().getStmts()) {
        if (md::is_type<Declaration>(arg.get())) {
          stmts.emplace_back(arg->clone());
        }
      }
      for (auto i = start->getValue(); i < end->getValue(); i += step->getValue()) {
        //auto body = forLoop.getBody().cloneBlock();
        stmts.emplace_back(std::make_unique<BinaryOp>(
          '=',
          std::make_unique<Variable>(forLoop.getIname()),
          std::make_unique<Number>(i)
        ));
        for (auto& arg : forLoop.getBody().getStmts()) {
          if (!md::is_type<Declaration>(arg.get())) {
            stmts.emplace_back(arg->clone());
          }
        }
      }
      return std::make_unique<Block>(std::move(stmts));
    }
    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_UNROLL_H_

