#ifndef PSPAMM_CODEGEN_H_
#define PSPAMM_CODEGEN_H_

#include <memory>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <optional>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "AST.h"
#include "visitor/FindModified.h"

using namespace llvm;

class CodeGen {
private:
  LLVMContext context;
  IRBuilder<> builder;
  std::unique_ptr<Module> module;
  using ValueMap = std::unordered_map<std::string, Value*>;
  std::vector<ValueMap> nv;

  Type* fpTy;
  Type* ptrToFpTy;
  Type* intTy;

  auto logError(char const* str) {
    std::cerr << "Error: " << str << std::endl;
    return nullptr;
  }

  auto findNV(std::string const& name) -> std::optional<std::reference_wrapper<Value*>> {
    for (auto it = nv.rbegin(); it != nv.rend(); ++it) {
      if (it->find(name) != it->end()) {
        return std::optional<std::reference_wrapper<Value*>>{(*it)[name]};
      }
    }
    return std::nullopt;
  }

public:
  CodeGen()
    : builder(context),
      module(std::make_unique<Module>("my cool jit", context)),
      fpTy(Type::getDoubleTy(context)),
      ptrToFpTy(PointerType::get(fpTy, 0)),
      intTy(IntegerType::get(context, 64)) {
    builder.setFastMathFlags(FastMathFlags::getFast());
  }

  Module& getModule() {
    return *module;
  }
  Type* operator()(Ty const& ty) {
    return nullptr;
  }

  Type* operator()(BasicTy const& ty) {
    Type* type = nullptr;
    switch (ty.getArTy()) {
      case ArTy::Int:
        type = IntegerType::get(context, ty.getNumBits());
        break;
      case ArTy::Float: {
        switch (ty.getNumBits()) {
          case 64:
            type = Type::getDoubleTy(context);
            break; 
          case 32:
            type = Type::getFloatTy(context);
            break;
          case 16:
            type = Type::getHalfTy(context);
            break;
          default:
            type = logError("Unknown floating point type");
        }
        break;
      }
    }
    if (ty.isPtrTy()) {
      return PointerType::get(type, 0);
    }
    return type;
  }

  /*  return_t operator()(Store& store) {
    Matrix& src = store.getSrc();

    VectorType* vecTy = VectorType::get(fpTy, src.getMb());
    PointerType* ptrToVecTy = PointerType::get(vecTy, 0);

    return_t rSrc = nv[src.getName()];
    Value* target = nv[store.getTarget().getName()][0];

    assert(rSrc.size() == src.getNb());
    assert(target != nullptr);

    for (int n = 0; n < src.getNb(); ++n) {
      Value* ptr = builder.CreateGEP(
          fpTy,
          target,
          ConstantInt::get(intTy, n*store.getLd()),
          "getptr");
      Value* vecPtr = builder.CreatePointerCast(ptr, ptrToVecTy, "tovecptr");
      builder.CreateStore(rSrc[n], vecPtr);
    }
    return return_t{nullptr};
  }


  return_t operator()(Outer& outer) {
    Matrix& a = outer.getA();
    Matrix& b = outer.getB();
    Matrix& c = outer.getC();

    return_t& rA = nv[a.getName()];
    return_t& rB = nv[b.getName()];
    return_t& rC = nv[c.getName()];

    assert(rA.size() > 0);
    assert(rB.size() > 0);
    assert(rC.size() > 0);

    assert(a.getMb() == c.getMb());
    assert(b.getNb() == c.getNb());
    assert(b.getMb() == 1);

    VectorType* vecTy = VectorType::get(fpTy, c.getMb());

    return_t block(b.getNb(), nullptr);
    for (int n = 0; n < b.getNb(); ++n) {
      Value* bbc = builder.CreateVectorSplat(c.getMb(), rB[n], "splat");
      const std::array<Type*, 3> types{vecTy, vecTy, vecTy};
      const std::array<Value*, 3> args{rA[0], bbc, rC[n]};
      block[n] = builder.CreateIntrinsic(Intrinsic::fmuladd, types, args, nullptr, "fma");
    } 
    return block;
  }

  return_t operator()(Incr& incr) {
    Value* var = nv[incr.getVar().getName()][0];
    return return_t{builder.CreateGEP(var, ConstantInt::get(intTy, incr.getIncr()), "movepointer")};
  }*/

  Value* operator()(Stmt&) {
    return logError("Not implemented");
  }


  Value* operator()(Call& call) {
    if (call.getCallee() == "load") {
      if (call.getArgs().size() != 3) {
        return logError("Load requires three arguments");
      }
      Value* first = md::visit(*this, *call.getArgs()[0]);
      Value* second = md::visit(*this, *call.getArgs()[1]);
      Value* third = md::visit(*this, *call.getArgs()[2]);
      auto* firstPtr = dyn_cast<PointerType>(first->getType());
      if (!firstPtr) {
        return logError("First argument to load must be pointer");
      }
      auto* thirdConst = dyn_cast<ConstantInt>(third);
      if (!thirdConst) {
        return logError("Third argument to load must be constant at compile time");
      }

      uint64_t vl = thirdConst->getValue().getLimitedValue();
      Type* loadTy = firstPtr->getElementType();

      Value* ptr = builder.CreateGEP(first, second, "getptr");
      if (vl > 1) {
        loadTy = VectorType::get(loadTy, vl);
        PointerType* ptrToVecTy = PointerType::get(loadTy, 0);
        ptr = builder.CreatePointerCast(ptr, ptrToVecTy, "tovecptr");
      }
      return builder.CreateAlignedLoad(loadTy, ptr, 1, "load");
    } else if (call.getCallee() == "store") {
      if (call.getArgs().size() != 3) {
        return logError("store requires three arguments");
      }
      Value* first = md::visit(*this, *call.getArgs()[0]);
      Value* second = md::visit(*this, *call.getArgs()[1]);
      Value* third = md::visit(*this, *call.getArgs()[2]);
      auto* secondPtr = dyn_cast<PointerType>(second->getType());
      if (!secondPtr) {
        return logError("Second argument to store must be pointer");
      }

      Type* storeTy = first->getType();
      Value* ptr = builder.CreateGEP(second, third, "getptr");
      PointerType* ptrToVecTy = PointerType::get(storeTy, 0);
      ptr = builder.CreatePointerCast(ptr, ptrToVecTy, "tovecptr");
      return builder.CreateAlignedStore(first, ptr, 1, "store");
    } else if (call.getCallee() == "splat") {
      if (call.getArgs().size() != 2) {
        return logError("Splat requires two arguments");
      }
      Value* first = md::visit(*this, *call.getArgs()[0]);
      Type* firstTy = first->getType();
      if (!firstTy->isFloatingPointTy() && !firstTy->isIntegerTy()) {
        return logError("Don't know how to splat type.");
      }
      Value* second = md::visit(*this, *call.getArgs()[1]);
      auto* secondConst = dyn_cast<ConstantInt>(second);
      if (!secondConst) {
        return logError("Second argument to splat must be constant at compile time");
      }
      return builder.CreateVectorSplat(secondConst->getValue().getLimitedValue(), first, "splat");
    }
    return logError("Unknown function");
  }

  Value* operator()(BinaryOp& binop) {
    if (binop.getOp() == '=') {
      Variable* lhsE = dynamic_cast<Variable*>(&binop.getLHS());
      if (!lhsE) {
        return logError("destination of '=' must be a variable");
      }
      Value* rhs = md::visit(*this, binop.getRHS());
      if (!rhs) {
        return nullptr;
      }
      auto lhs = findNV(lhsE->getName());
      if (!lhs) {
        return logError("Unknown variable name");
      }
      lhs.value().get() = rhs;
      return rhs;
    }

    Value* lhs = md::visit(*this, binop.getLHS());
    Value* rhs = md::visit(*this, binop.getRHS());

    if (!lhs || !rhs) {
      return nullptr;
    }

    if (lhs->getType()->getScalarType()->isFloatingPointTy()) {
      switch (binop.getOp()) {
        case '+':
          return builder.CreateFAdd(lhs, rhs, "addtmp");
        case '-':
          return builder.CreateFSub(lhs, rhs, "subtmp");
        case '*':
          return builder.CreateFMul(lhs, rhs, "multmp");
        default:
          return logError("Unknown operator");
      }
    }
    switch (binop.getOp()) {
      case '+':
        return builder.CreateAdd(lhs, rhs, "addtmp");
      case '-':
        return builder.CreateSub(lhs, rhs, "subtmp");
      case '*':
        return builder.CreateMul(lhs, rhs, "multmp");
      default:
        return logError("Unknown operator");
    }
    return nullptr;
  }

  Value* operator()(Variable& var) {
    auto vnv = findNV(var.getName());
    if (!vnv) {
      std::stringstream err;
      err << "Variable " << var.getName() << " not declared";
      return logError(err.str().c_str());
    }
    Value* v = vnv.value().get();
    if (!v) {
      std::stringstream err;
      err << "Variable " << var.getName() << " not defined";
      return logError(err.str().c_str());
    }
    return v;
  }

  Value* operator()(Number& number) {
    Type* type = md::visit(*this, *number.getTy());
    return ConstantInt::get(type, number.getValue());
  }

  Value* operator()(Declaration& decl) {
    if (nv.back().find(decl.getName()) != nv.back().end()) {
      return logError("Variable already declared in local scope");
    }
    nv.back()[decl.getName()] = nullptr;
    return nullptr;
  }

  Value* operator()(Block& block) {
    nv.push_back(ValueMap{});
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    } 
    nv.pop_back();
    return nullptr;
  }

  Value* operator()(For& forLoop) {
    FindModified modfd;
    md::visit(modfd, forLoop.getBody());

    Value* start = md::visit(*this, forLoop.getStart());

    Function* f = builder.GetInsertBlock()->getParent();
    BasicBlock* bb = builder.GetInsertBlock();
    BasicBlock* loopBB = BasicBlock::Create(context, "loop", f);
    builder.CreateBr(loopBB);
    builder.SetInsertPoint(loopBB);

    std::unordered_map<std::string, PHINode*> phiMap;
    for (auto& mod : modfd.getVars()) {
      auto vnv = findNV(mod);
      if (vnv) {
        Value*& v = vnv.value().get();
        PHINode* phi = builder.CreatePHI(v->getType(), 2, mod.c_str());
        phi->addIncoming(v, bb);
        phiMap[mod] = phi;
        v = phi;
      }
    }

    auto iname = forLoop.getIname();
    PHINode* phi = builder.CreatePHI(intTy, 2, "loopvarphi");
    phi->addIncoming(start, bb);
    Value* oldIname = nullptr;
    if (nv.back().find(iname) != nv.back().end()) {
      oldIname = nv.back().at(iname);
    }
    nv.back()[iname] = phi;

    md::visit(*this, forLoop.getBody());

    Value* end = md::visit(*this, forLoop.getEnd());
    Value* step = md::visit(*this, forLoop.getStep());

    Value* nextPhi = builder.CreateAdd(phi, step, "nextvar");
    Value* cmp = builder.CreateICmpSLT(nextPhi, end, "icmp");

    BasicBlock* loopEndBB = builder.GetInsertBlock();
    BasicBlock* afterBB = BasicBlock::Create(context, "afterloop", f);
    builder.CreateCondBr(cmp, loopBB, afterBB);
    builder.SetInsertPoint(afterBB);
    phi->addIncoming(nextPhi, loopEndBB);

    if (oldIname) {
      nv.back()[iname] = oldIname;
    } else {
      nv.back().erase(iname);
    }

    for (auto& entry : phiMap) {
      Value* v = findNV(entry.first).value().get();
      entry.second->addIncoming(v, loopEndBB);
    }

    return phi;
  }

  /*return_t operator()(AnonymousFunction& function) {
    std::vector<Type*> ptrs(function.getArgs().size(), ptrToFpTy);
    FunctionType* ft = FunctionType::get(Type::getVoidTy(context), ptrs, false);
    Function* f = Function::Create(ft, Function::ExternalLinkage, "", module.get());

    BasicBlock* bb = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(bb);
  
    auto nameIt = function.getArgs().begin();
    for (auto& arg : f->args()) {
      arg.setName(*nameIt);
      nv[*nameIt] = return_t{&arg};
      ++nameIt;
    }

    md::visit(*this, function.getBody());

    builder.CreateRetVoid();
    verifyFunction(*f);

    return return_t{f};
  }*/
  void generate(Stmt& stmt) {
    std::vector<Type*> ptrs(3, ptrToFpTy);
    FunctionType* ft = FunctionType::get(Type::getVoidTy(context), ptrs, false);
    Function* f = Function::Create(ft, Function::ExternalLinkage, "", module.get());

    nv.clear();
    nv.push_back(ValueMap{});
    std::vector<std::string> names{"A", "B", "C"};
    auto nameIt = names.begin();
    for (auto& arg : f->args()) {
      arg.setName(*nameIt);
      nv.back()[*nameIt] = &arg;
      ++nameIt;
    }

    BasicBlock* bb = BasicBlock::Create(context, "entry", f);
    builder.SetInsertPoint(bb);

    md::visit(*this, stmt);

    builder.CreateRetVoid();
    verifyFunction(*f);
  }
};

#endif // PSPAMM_CODEGEN_H_
