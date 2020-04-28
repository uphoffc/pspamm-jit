#ifndef PSPAMM_VISITOR_TILE_H_
#define PSPAMM_VISITOR_TILE_H_

#include <cstdint>
#include <algorithm>
#include "AST.h"
#include "Subs.h"

class Tile {
private:
  std::string iname;
  int64_t maxTripCount;
  bool maximise;

public:
  Tile(std::string const& iname, int64_t maxTripCount, bool maximise = false)
    : iname(iname), maxTripCount(maxTripCount), maximise(maximise) {}

  std::unique_ptr<Block> operator()(Stmt& stmt) { return nullptr; }

  std::unique_ptr<Block> operator()(Block& block) {
    for (auto& stmt : block.getStmts()) {
      auto newStmt = md::visit(*this, *stmt);
      if (newStmt) {
        if (block.getStmts().size() == 1) {
          return std::move(newStmt);
        } else {
          stmt = std::move(newStmt);
        }
      }
    }
    return nullptr;
  }

  std::unique_ptr<Block> operator()(For& forLoop) {
    auto newBody = md::visit(*this, forLoop.getBody());
    if (newBody) {
      forLoop.setBody(std::move(newBody));
    }

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
      int64_t repeat1, repeat2, newTripCount[2];
      if (maximise) {
        newTripCount[0] = std::min(maxTripCount, tripCount);
        repeat1 = tripCount / newTripCount[0];
        repeat2 = 1;
        newTripCount[1] = tripCount - repeat1*newTripCount[0];
      } else {
        auto numTiles = 1 + (tripCount-1) / maxTripCount;
        newTripCount[0] = 1 + (tripCount-1) / numTiles;
        newTripCount[1] = tripCount / numTiles;
        repeat1 = tripCount % newTripCount[1];
        repeat2 = (tripCount - repeat1 * newTripCount[0]) / newTripCount[1];
      }
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
            BinaryOp('+',
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

#endif // PSPAMM_VISITOR_TILE_H_

