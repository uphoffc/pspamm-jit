#include <iostream>
#include <sstream>
#include <md/visit.hpp>

#include "Parser.h"
#include "PrettyPrinter.h"
#include "CodeGen.h"
#include "visitor/CheckTypes.h"
#include "visitor/FoldConstants.h"
#include "visitor/Unroll.h"
#include "visitor/Tile.h"
#include "visitor/MoveLoop.h"
#include "visitor/Vectorize.h"

int main() {
  std::string code(
R"(
{
  var M: i64;
  var N: i64;
  var K: i64;
  var ldA: i64;
  var ldB: i64;
  var ldC: i64;
  M = 16;
  N = 61-1;
  K = 16;
  ldA = M;
  ldB = K;
  ldC = M;
  for n=0,N,30 {
    for m=0,M,8 {
      var c0: <f64:8>;
      var c1: <f64:8>;
      var c2: <f64:8>;
      var c3: <f64:8>;
      var c4: <f64:8>;
      var c5: <f64:8>;
      var c6: <f64:8>;
      var c7: <f64:8>;
      var c8: <f64:8>;
      var c9: <f64:8>;
      var c10: <f64:8>;
      var c11: <f64:8>;
      var c12: <f64:8>;
      var c13: <f64:8>;
      var c14: <f64:8>;
      var c15: <f64:8>;
      var c16: <f64:8>;
      var c17: <f64:8>;
      var c18: <f64:8>;
      var c19: <f64:8>;
      var c20: <f64:8>;
      var c21: <f64:8>;
      var c22: <f64:8>;
      var c23: <f64:8>;
      var c24: <f64:8>;
      var c25: <f64:8>;
      var c26: <f64:8>;
      var c27: <f64:8>;
      var c28: <f64:8>;
      var c29: <f64:8>;
      c0 = load(C,m+n*ldC+0*ldC,8);
      c1 = load(C,m+n*ldC+1*ldC,8);
      c2 = load(C,m+n*ldC+2*ldC,8);
      c3 = load(C,m+n*ldC+3*ldC,8);
      c4 = load(C,m+n*ldC+4*ldC,8);
      c5 = load(C,m+n*ldC+5*ldC,8);
      c6 = load(C,m+n*ldC+6*ldC,8);
      c7 = load(C,m+n*ldC+7*ldC,8);
      c8 = load(C,m+n*ldC+8*ldC,8);
      c9 = load(C,m+n*ldC+9*ldC,8);
      c10 = load(C,m+n*ldC+10*ldC,8);
      c11 = load(C,m+n*ldC+11*ldC,8);
      c12 = load(C,m+n*ldC+12*ldC,8);
      c13 = load(C,m+n*ldC+13*ldC,8);
      c14 = load(C,m+n*ldC+14*ldC,8);
      c15 = load(C,m+n*ldC+15*ldC,8);
      c16 = load(C,m+n*ldC+16*ldC,8);
      c17 = load(C,m+n*ldC+17*ldC,8);
      c18 = load(C,m+n*ldC+18*ldC,8);
      c19 = load(C,m+n*ldC+19*ldC,8);
      c20 = load(C,m+n*ldC+20*ldC,8);
      c21 = load(C,m+n*ldC+21*ldC,8);
      c22 = load(C,m+n*ldC+22*ldC,8);
      c23 = load(C,m+n*ldC+23*ldC,8);
      c24 = load(C,m+n*ldC+24*ldC,8);
      c25 = load(C,m+n*ldC+25*ldC,8);
      c26 = load(C,m+n*ldC+26*ldC,8);
      c27 = load(C,m+n*ldC+27*ldC,8);
      c28 = load(C,m+n*ldC+28*ldC,8);
      c29 = load(C,m+n*ldC+29*ldC,8);
      for k=0,K {
        var a: <f64:8>;
        var b0: f64;
        var b1: f64;
        var b2: f64;
        var b3: f64;
        var b4: f64;
        var b5: f64;
        var b6: f64;
        var b7: f64;
        var b8: f64;
        var b9: f64;
        var b10: f64;
        var b11: f64;
        var b12: f64;
        var b13: f64;
        var b14: f64;
        var b15: f64;
        var b16: f64;
        var b17: f64;
        var b18: f64;
        var b19: f64;
        var b20: f64;
        var b21: f64;
        var b22: f64;
        var b23: f64;
        var b24: f64;
        var b25: f64;
        var b26: f64;
        var b27: f64;
        var b28: f64;
        var b29: f64;
        a = load(A,m+k*ldA,8);
        b0 = load(B,k+n*ldB+0*ldB,1);
        b1 = load(B,k+n*ldB+1*ldB,1);
        b2 = load(B,k+n*ldB+2*ldB,1);
        b3 = load(B,k+n*ldB+3*ldB,1);
        b4 = load(B,k+n*ldB+4*ldB,1);
        b5 = load(B,k+n*ldB+5*ldB,1);
        b6 = load(B,k+n*ldB+6*ldB,1);
        b7 = load(B,k+n*ldB+7*ldB,1);
        b8 = load(B,k+n*ldB+8*ldB,1);
        b9 = load(B,k+n*ldB+9*ldB,1);
        b10 = load(B,k+n*ldB+10*ldB,1);
        b11 = load(B,k+n*ldB+11*ldB,1);
        b12 = load(B,k+n*ldB+12*ldB,1);
        b13 = load(B,k+n*ldB+13*ldB,1);
        b14 = load(B,k+n*ldB+14*ldB,1);
        b15 = load(B,k+n*ldB+15*ldB,1);
        b16 = load(B,k+n*ldB+16*ldB,1);
        b17 = load(B,k+n*ldB+17*ldB,1);
        b18 = load(B,k+n*ldB+18*ldB,1);
        b19 = load(B,k+n*ldB+19*ldB,1);
        b20 = load(B,k+n*ldB+20*ldB,1);
        b21 = load(B,k+n*ldB+21*ldB,1);
        b22 = load(B,k+n*ldB+22*ldB,1);
        b23 = load(B,k+n*ldB+23*ldB,1);
        b24 = load(B,k+n*ldB+24*ldB,1);
        b25 = load(B,k+n*ldB+25*ldB,1);
        b26 = load(B,k+n*ldB+26*ldB,1);
        b27 = load(B,k+n*ldB+27*ldB,1);
        b28 = load(B,k+n*ldB+28*ldB,1);
        b29 = load(B,k+n*ldB+29*ldB,1);
        c0 = c0 + a*b0;
        c1 = c1 + a*b1;
        c2 = c2 + a*b2;
        c3 = c3 + a*b3;
        c4 = c4 + a*b4;
        c5 = c5 + a*b5;
        c6 = c6 + a*b6;
        c7 = c7 + a*b7;
        c8 = c8 + a*b8;
        c9 = c9 + a*b9;
        c10 = c10 + a*b10;
        c11 = c11 + a*b11;
        c12 = c12 + a*b12;
        c13 = c13 + a*b13;
        c14 = c14 + a*b14;
        c15 = c15 + a*b15;
        c16 = c16 + a*b16;
        c17 = c17 + a*b17;
        c18 = c18 + a*b18;
        c19 = c19 + a*b19;
        c20 = c20 + a*b20;
        c21 = c21 + a*b21;
        c22 = c22 + a*b22;
        c23 = c23 + a*b23;
        c24 = c24 + a*b24;
        c25 = c25 + a*b25;
        c26 = c26 + a*b26;
        c27 = c27 + a*b27;
        c28 = c28 + a*b28;
        c29 = c29 + a*b29;
      }
      store(c0,C,m+n*ldC+0*ldC);
      store(c1,C,m+n*ldC+1*ldC);
      store(c2,C,m+n*ldC+2*ldC);
      store(c3,C,m+n*ldC+3*ldC);
      store(c4,C,m+n*ldC+4*ldC);
      store(c5,C,m+n*ldC+5*ldC);
      store(c6,C,m+n*ldC+6*ldC);
      store(c7,C,m+n*ldC+7*ldC);
      store(c8,C,m+n*ldC+8*ldC);
      store(c9,C,m+n*ldC+9*ldC);
      store(c10,C,m+n*ldC+10*ldC);
      store(c11,C,m+n*ldC+11*ldC);
      store(c12,C,m+n*ldC+12*ldC);
      store(c13,C,m+n*ldC+13*ldC);
      store(c14,C,m+n*ldC+14*ldC);
      store(c15,C,m+n*ldC+15*ldC);
      store(c16,C,m+n*ldC+16*ldC);
      store(c17,C,m+n*ldC+17*ldC);
      store(c18,C,m+n*ldC+18*ldC);
      store(c19,C,m+n*ldC+19*ldC);
      store(c20,C,m+n*ldC+20*ldC);
      store(c21,C,m+n*ldC+21*ldC);
      store(c22,C,m+n*ldC+22*ldC);
      store(c23,C,m+n*ldC+23*ldC);
      store(c24,C,m+n*ldC+24*ldC);
      store(c25,C,m+n*ldC+25*ldC);
      store(c26,C,m+n*ldC+26*ldC);
      store(c27,C,m+n*ldC+27*ldC);
      store(c28,C,m+n*ldC+28*ldC);
      store(c29,C,m+n*ldC+29*ldC);
    }
  }
}
)");
  std::string code2(
R"(
{
  var M: i64;
  var N: i64;
  var K: i64;
  var ldA: i64;
  var ldB: i64;
  var ldC: i64;
  M = 16;
  N = 60;
  K = 16;
  ldA = 42;
  ldB = 64;
  ldC = 56;
  for n=0,N,6 {
    for m=0,M,4 {
      var c0: <f64:4>;
      var c1: <f64:4>;
      var c2: <f64:4>;
      var c3: <f64:4>;
      var c4: <f64:4>;
      var c5: <f64:4>;
      c0 = load(C,m+n*ldC+0*ldC,4);
      c1 = load(C,m+n*ldC+1*ldC,4);
      c2 = load(C,m+n*ldC+2*ldC,4);
      c3 = load(C,m+n*ldC+3*ldC,4);
      c4 = load(C,m+n*ldC+4*ldC,4);
      c5 = load(C,m+n*ldC+5*ldC,4);
      for k=0,K {
        var a: <f64:4>;
        var b0: f64;
        var b1: f64;
        var b2: f64;
        var b3: f64;
        var b4: f64;
        var b5: f64;
        var b6: f64;
        var b7: f64;
        a = load(A,m+k*ldA,4);
        b0 = load(B,k+n*ldB+0*ldB,1);
        b1 = load(B,k+n*ldB+1*ldB,1);
        b2 = load(B,k+n*ldB+2*ldB,1);
        b3 = load(B,k+n*ldB+3*ldB,1);
        b4 = load(B,k+n*ldB+4*ldB,1);
        b5 = load(B,k+n*ldB+5*ldB,1);
        c0 = c0 + a*b0;
        c1 = c1 + a*b1;
        c2 = c2 + a*b2;
        c3 = c3 + a*b3;
        c4 = c4 + a*b4;
        c5 = c5 + a*b5;
      }
      store(c0,C,m+n*ldC+0*ldC);
      store(c1,C,m+n*ldC+1*ldC);
      store(c2,C,m+n*ldC+2*ldC);
      store(c3,C,m+n*ldC+3*ldC);
      store(c4,C,m+n*ldC+4*ldC);
      store(c5,C,m+n*ldC+5*ldC);
    }
  }
}
)");
  std::string code3(
R"(
{
  var M: i64;
  var N: i64;
  var K: i64;
  var ldA: i64;
  var ldB: i64;
  var ldC: i64;
  M = 16;
  N = 60;
  K = 4;
  ldA = M;
  ldB = K;
  ldC = M;
  for n=0,N {
    for m=0,M {
      for k=0,K {
        var a: f64;
        var b: f64;
        var c: f64;
        a = load<1>(A,m+k*ldA);
        b = load<1>(B,k+n*ldB);
        c = load<1>(C,m+n*ldC);
        c = c + a*b;
        store(c,C,m+n*ldC);
      }
    }
  }
}
)");
  std::stringstream codeStream(code3);
  Lexer lexer(codeStream);
  Parser parser(lexer);
  parser.getNextToken();
  auto ast = parser.parseStatement();
  //auto ast = generate(16, 30, 4);

  md::visit(CheckTypes{}, *ast);
  md::visit(FoldConstants{}, *ast);
  md::visit(Tile("n", 30), *ast);
  md::visit(MoveLoop("n"), *ast);
  md::visit(Tile("m", 8, true), *ast);
  md::visit(MoveLoop("m"), *ast);
  md::visit(Vectorize("m"), *ast);
  md::visit(Unroll("n"), *ast);

  md::visit(PrettyPrinter(std::cout), *ast);
  std::cout << std::endl;

  CodeGen cg;
  cg.generate(*ast);
  //md::visit(cg, *ast);
  cg.getModule().print(llvm::errs(), nullptr);

  //CodeGen cg;
  //auto f = generate(16, 8, 4);
  ////f->print(llvm::errs());
  //cg.getModule().print(llvm::errs(), nullptr);

  return 0;
}
