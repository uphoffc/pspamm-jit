#ifndef PSPAMM_CODEGEN_H_
#define PSPAMM_CODEGEN_H_

#include <memory>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "AST.h"
#include "FindReadVars.h"

using namespace llvm;

class CodeGen {
private:
  LLVMContext context;
  IRBuilder<> builder;
  std::unique_ptr<Module> module;
  std::unordered_map<std::string, std::vector<Value*>> nv;

  Type* fpTy;
  Type* ptrToFpTy;
  Type* intTy;

  auto logError(char const* str) {
    std::cerr << "Error: " << str << std::endl;
    return nullptr;
  }

public:
  using return_t = std::vector<Value*>;

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

  return_t operator()(Stmt& stmt) {
    return return_t{nullptr};
  }

  /*

  void go_steps(std::string const& name, int steps) {
   nv[name] = builder.CreateGEP(
        fpTy,
        nv[name],
        ConstantInt::get(intTy, steps),
        "movecursor");
  }

  void far_up(Cursor& c) {
    int steps = c.far_up();
    go_steps(c.getName(), steps);
  }

  void far_left(Cursor& c) {
    int steps = c.far_up();
    go_steps(c.getName(), steps * c.rows());
  }

  void down(Cursor& c) {
    int steps = c.down();
    go_steps(c.getName(), steps);
  }

  void right(Cursor& c) {
    int steps = c.right();
    go_steps(c.getName(), steps * c.rows());
  }

  template<typename Lambda>
  void unroll(int start, int end, int step, Lambda body, std::vector<std::string>&&) {
    for (int i = start; i < end; i += step) {
      body();
    }
  }*/
  return_t operator()(Load& load) {
    return_t block(load.getNb(), nullptr);

    VectorType* vecTy = VectorType::get(fpTy, load.getMb());
    PointerType* ptrToVecTy = PointerType::get(vecTy, 0);

    Value* base = nv[load.getSrc().getName()][0];
    assert(base != nullptr);

    for (int n = 0; n < load.getNb(); ++n) {
      Value* ptr = builder.CreateGEP(
          fpTy,
          base,
          ConstantInt::get(intTy, n*load.getLd()),
          "getptr");
      Type* loadTy = fpTy;
      if (load.getMb() > 1) {
        ptr = builder.CreatePointerCast(ptr, ptrToVecTy, "tovecptr");
        loadTy = vecTy;
      }
      block[n] = builder.CreateLoad(loadTy, ptr, "load");
    }
    return block;
  }

  return_t operator()(Store& store) {
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
  }

  return_t operator()(Assign& assign) {
    auto v = md::visit(*this, assign.getRHS());
    nv[assign.getLHS().getName()] = v;
    return v;
  }

  return_t operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      md::visit(*this, *stmt);
    } 
    return return_t{nullptr};
  }

  return_t operator()(For& forLoop) {
    FindReadVars frv;
    md::visit(frv, forLoop.getBody());

    Function* f = builder.GetInsertBlock()->getParent();
    BasicBlock* bb = builder.GetInsertBlock();
    BasicBlock* loopBB = BasicBlock::Create(context, "loop", f);
    builder.CreateBr(loopBB);
    builder.SetInsertPoint(loopBB);

    PHINode* phi = builder.CreatePHI(intTy, 2, "loopvarphi");
    phi->addIncoming(ConstantInt::get(intTy, forLoop.getStart()), bb);

    std::unordered_map<std::string, std::vector<PHINode*>> phiMap;
    for (auto& rv : frv.getVars()) {
      if (nv.find(rv) != nv.end()) {
        auto& vs = nv[rv];
        phiMap[rv] = std::vector<PHINode*>{};
        for (auto& v : vs) {
          PHINode* phi = builder.CreatePHI(v->getType(), 2, rv.c_str());
          phi->addIncoming(v, bb);
          v = phi;
          phiMap[rv].emplace_back(phi);
        }
      }
    }

    md::visit(*this, forLoop.getBody());
      
    Value* nextPhi = builder.CreateAdd(phi, ConstantInt::get(intTy, forLoop.getStep()), "nextvar");
    Value* cmp = builder.CreateICmpSLT(nextPhi, ConstantInt::get(intTy, forLoop.getStop()), "icmp");

    BasicBlock* loopEndBB = builder.GetInsertBlock();
    BasicBlock* afterBB = BasicBlock::Create(context, "afterloop", f);
    builder.CreateCondBr(cmp, loopBB, afterBB);
    builder.SetInsertPoint(afterBB);
    phi->addIncoming(nextPhi, loopEndBB);

    for (auto& entry : phiMap) {
      auto vit = nv[entry.first].begin();
      for (auto& phi : entry.second) {
        phi->addIncoming(*vit, loopEndBB);
        ++vit;
      }
    }

    return return_t{phi};
  }

  return_t operator()(AnonymousFunction& function) {
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
  }
};

#endif // PSPAMM_CODEGEN_H_
