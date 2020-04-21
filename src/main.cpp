#include <iostream>
#include <md/visit.hpp>

#include "ASTGenerator.h"
#include "PrettyPrinter.h"
#include "CodeGen.h"

int main() {
  auto ast = generate(16, 30, 4);

  md::visit(PrettyPrinter(std::cout), *ast);

  CodeGen cg;
  md::visit(cg, *ast);
  cg.getModule().print(llvm::errs(), nullptr);

  //CodeGen cg;
  //auto f = generate(16, 8, 4);
  ////f->print(llvm::errs());
  //cg.getModule().print(llvm::errs(), nullptr);

  return 0;
}
