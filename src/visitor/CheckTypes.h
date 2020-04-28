#ifndef PSPAMM_VISITOR_CHECKTYPES_H_
#define PSPAMM_VISITOR_CHECKTYPES_H_

#include <unordered_map>
#include <string>
#include <sstream>

#include "AST.h"
#include "Type.h"
#include "CommonTy.h"
#include "Traverse.h"

class CheckTypes : public Traverse<CheckTypes,Ty const*> {
private:
  using TypeMap = std::unordered_map<std::string, Ty const*>;
  std::vector<TypeMap> types;

  return_t logError(char const* str) {
    std::cerr << "CheckTypes error: " << str << std::endl;
    return nullptr;
  }

  return_t logError(std::string const& str) {
    return logError(str.c_str());
  }

public:
  using Traverse<CheckTypes,return_t>::operator();

  return_t operator()(Declaration& decl) {
    if (types.back().find(decl.getName()) != types.back().end()) {
      return logError("Variable already declared in local scope");
    }
    types.back()[decl.getName()] = &decl.getTy();
    return &decl.getTy();
  }

  return_t operator()(Variable& var) {
    for (auto it = types.rbegin(); it != types.rend(); ++it) {
      if (it->find(var.getName()) != it->end()) {
        auto ty = it->at(var.getName()); 
        var.setTy(ty);
        return ty;
      }
    }
    std::stringstream err;
    err << "Could not find type for variable " << var.getName();
    //return logError(err.str());
    return nullptr;
  }

  return_t operator()(Number& number) {
    return number.getTy();
  }

  return_t operator()(BinaryOp& op) {
    auto lhs = md::visit(*this, op.getLHS());
    auto rhs = md::visit(*this, op.getRHS());

    if (!lhs || !rhs) {
      return nullptr;
    }

    Ty const* common = md::visit(CommonTy{}, *lhs, *rhs);
    if (!common) {
      return nullptr;
    }

    auto splat = [](std::unique_ptr<Expr> expr, int64_t vl) {
      std::vector<std::unique_ptr<Expr>> args;
      args.emplace_back(std::move(expr));
      return std::make_unique<Call>("splat", std::move(args), std::vector<int64_t>{vl});
    };
    if (common->isVectorTy()) {
      if (!lhs->isVectorTy()) {
        op.setLHS( splat(op.transLHS(), static_cast<VectorTy const*>(common)->getNumElements()) );
      }
      if (!rhs->isVectorTy()) {
        op.setRHS( splat(op.transRHS(), static_cast<VectorTy const*>(common)->getNumElements()) );
      }
    }
    op.setTy(common);

    return return_t{};
  }

  return_t operator()(Call& call) {
    for (auto& arg : call.getArgs()) {
      md::visit(*this, *arg);
    }
    // TODO set type
    return return_t{};
  }

  return_t operator()(Block& block) {
    types.push_back(TypeMap{});
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    }
    types.pop_back();
    return return_t{};
  }
};

#endif // PSPAMM_VISITOR_CHECKTYPES_H_

