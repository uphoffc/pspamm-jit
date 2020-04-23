#ifndef PSPAMM_VISITOR_MOVELOOP_H_
#define PSPAMM_VISITOR_MOVELOOP_H_

#include <cstdint>
#include "AST.h"
#include "Subs.h"

class MoveLoop {
private:
  std::string iname;

public:
  MoveLoop(std::string const& iname)
    : iname(iname) {}

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

    if (iname == forLoop.getIname()) {
      auto start = dynamic_cast<Number const*>(&forLoop.getStart());
      auto end = dynamic_cast<Number const*>(&forLoop.getEnd());
      auto step = dynamic_cast<Number const*>(&forLoop.getStep());
      if (!start || !end || !step) {
        std::cout << "Can only tile loops with constant limits" << std::endl;
        return nullptr;
      }
      auto tripCount = end->getValue() - start->getValue();
      if (tripCount <= maxTripCount || tripCount <= 0 || tripCount % step->getValue() != 0) {
        return nullptr;
      }
      //if (tripCount % maxTripCount == 0) {
        //std::vector<std::unique_ptr<Stmt>> stmts;
        //stmts.emplace_back(std::make_unique<For>(
          //iname + "_inner",
          //std::make_unique<Variable>(iname),
          //std::make_unique<BinaryOp>('+',
            //std::make_unique<Variable>(iname),
            //std::make_unique<Number>(maxTripCount)
          //),
          //step->cloneExpr(),
          //forLoop.transBody()
        //));
        //forLoop.setStep(std::make_unique<Number>(maxTripCount*step->getValue()));
        //forLoop.setBody(std::make_unique<Block>(std::move(stmts)));
      //}
      // numTiles = ceil(tripCount/maxTripCount)
      auto numTiles = 1 + (tripCount-1) / maxTripCount;
      int64_t newTripCount[]{
        1 + (tripCount-1) / numTiles,
        tripCount / numTiles
      };
      auto repeat1 = tripCount % newTripCount[1];
      auto repeat2 = (tripCount - repeat1 * newTripCount[0]) / newTripCount[1];
      std::vector<std::unique_ptr<Stmt>> stmts;
      auto addTile = [&](std::string&& newIname,
                         int64_t newTripCount,
                         std::unique_ptr<Number> newStart,
                         std::unique_ptr<Number> newEnd,
                         std::unique_ptr<Number> newStep) {
        if (newStart->getValue() < newEnd->getValue()) {
          std::vector<std::unique_ptr<Stmt>> stmtsInner;
          std::string innerIname(forLoop.getIname());
          auto body = forLoop.getBody().cloneBlock();
          md::visit(Subs(forLoop.getIname(), 
            std::make_unique<BinaryOp>('+',
              std::make_unique<Variable>(newIname),
              std::make_unique<Variable>(innerIname)
            )), *body);
          stmtsInner.emplace_back(std::make_unique<For>(
            innerIname,
            std::make_unique<Number>(0),
            std::make_unique<Number>(newTripCount),
            step->cloneExpr(),
            std::move(body)
          ));
          stmts.emplace_back(std::make_unique<For>(
            newIname,
            std::move(newStart),
            std::move(newEnd),
            std::move(newStep),
            std::make_unique<Block>(std::move(stmtsInner))
          ));
        }
      };
      addTile(iname + "_head",
              newTripCount[0],
              std::make_unique<Number>(start->getValue()),
              std::make_unique<Number>(start->getValue() + repeat1*newTripCount[0]),
              std::make_unique<Number>(newTripCount[0]*step->getValue()));
      addTile(iname + "_tail",
              newTripCount[1],
              std::make_unique<Number>(start->getValue() + repeat1*newTripCount[0]),
              std::make_unique<Number>(end->getValue()),
              std::make_unique<Number>(newTripCount[1]*step->getValue()));
      return std::make_unique<Block>(std::move(stmts));
    }
    return nullptr;
  }
};

#endif // PSPAMM_VISITOR_MOVELOOP_H_

