#ifndef PSPAMM_ASTGENERATOR_H_
#define PSPAMM_ASTGENERATOR_H_

#include <memory>
#include <vector>

#include "AST.h"

namespace internal {

std::unique_ptr<Stmt> kLoop(int Mb, int Nb, int K, int ldA, int ldB) {
  std::vector<std::unique_ptr<Stmt>> stmts;
  for (int k = 0; k < K; ++k) {
    stmts.emplace_back(
      std::make_unique<Assign>(
        std::make_unique<Matrix>("a", Mb, 1),
        std::make_unique<Load>(std::make_unique<Ptr>("A"), Mb, 1, ldA)
      )
    );
    stmts.emplace_back(
      std::make_unique<Assign>(
        std::make_unique<Matrix>("b", 1, Nb),
        std::make_unique<Load>(std::make_unique<Ptr>("B"), 1, Nb, ldB)
      )
    );
    stmts.emplace_back(
      std::make_unique<Assign>(
        std::make_unique<Matrix>("c", Mb, Nb),
        std::make_unique<Outer>(
          std::make_unique<Matrix>("a", Mb, 1),
          std::make_unique<Matrix>("b", 1, Nb),
          std::make_unique<Matrix>("c", Mb, Nb)
        )
      )
    );
    stmts.emplace_back(
      std::make_unique<Assign>(
        std::make_unique<Ptr>("A"),
        std::make_unique<Incr>(std::make_unique<Ptr>("A"), ldA)
      )
    );
    stmts.emplace_back(
      std::make_unique<Assign>(
        std::make_unique<Ptr>("B"),
        std::make_unique<Incr>(std::make_unique<Ptr>("B"), 1)
      )
    );
  }
  return std::make_unique<Block>(std::move(stmts));
  /*return std::make_unique<For>(
      0,
      K,
      1,
      std::make_unique<Block>(std::move(stmts))
  );*/
}

std::unique_ptr<Stmt> mLoop(int M0, int M1, int Mb, int Nb, int K, int ldA, int ldB, int ldC) {
  std::vector<std::unique_ptr<Stmt>> stmts;
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Matrix>("c", Mb, Nb),
      std::make_unique<Load>(std::make_unique<Ptr>("C"), Mb, Nb, ldC)
    )
  );
  stmts.emplace_back(
    internal::kLoop(Mb, Nb, K, ldA, ldB)
  );
  stmts.emplace_back(
    std::make_unique<Store>(
      std::make_unique<Matrix>("c", Mb, Nb),
      std::make_unique<Ptr>("C"),
      ldC
    )
  );
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Ptr>("A"),
      std::make_unique<Incr>(std::make_unique<Ptr>("A"), Mb - K*ldA)
    )
  );
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Ptr>("B"),
      std::make_unique<Incr>(std::make_unique<Ptr>("B"), -K)
    )
  );
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Ptr>("C"),
      std::make_unique<Incr>(std::make_unique<Ptr>("C"), Mb)
    )
  );
  return std::make_unique<For>(
      M0,
      M1,
      Mb,
      std::make_unique<Block>(std::move(stmts))
  );
}

std::unique_ptr<Stmt> nLoop(int M0, int M1, int Mb, int N0, int N1, int Nb, int K, int ldA, int ldB, int ldC) {
  std::vector<std::unique_ptr<Stmt>> stmts;
  stmts.emplace_back(
    internal::mLoop(M0, M1, Mb, Nb, K, ldA, ldB, ldC)
  );
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Ptr>("A"),
      std::make_unique<Incr>(std::make_unique<Ptr>("A"), -M1)
    )
  );
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Ptr>("B"),
      std::make_unique<Incr>(std::make_unique<Ptr>("B"), Nb*ldB - M1)
    )
  );
  stmts.emplace_back(
    std::make_unique<Assign>(
      std::make_unique<Ptr>("C"),
      std::make_unique<Incr>(std::make_unique<Ptr>("C"), Nb*ldC)
    )
  );
  return std::make_unique<For>(
      N0,
      N1,
      Nb,
      std::make_unique<Block>(std::move(stmts))
  );
}

}

std::unique_ptr<Stmt> generate(int M, int N, int K) {
  return std::make_unique<AnonymousFunction>(
    std::vector<std::string>{"A", "B", "C"},
    internal::nLoop(0, M, 8, 0, N, N, K, M, K, M)
  );
}

#endif // PSPAMM_ASTGENERATOR_H_
