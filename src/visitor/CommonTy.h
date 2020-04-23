#ifndef PSPAMM_VISITOR_COMMONTYPE_H_
#define PSPAMM_VISITOR_COMMONTYPE_H_

#include "Type.h"

class CommonTy {
private:
  Ty const* logError(char const* str) {
    std::cerr << "CommonType error: " << str << std::endl;
    return nullptr;
  }

public:
  Ty const* operator()(BasicTy const& lhs, BasicTy const& rhs) {
    if (lhs.isPtrTy() || rhs.isPtrTy()) {
      return logError("If you want C then use C! (Pointer arithmetic not allowed.)");
    }
    if (lhs.getArTy() == rhs.getArTy() && lhs.getNumBits() == rhs.getNumBits()) {
      return &lhs;
    }
    return logError("Mixed type or mixed precision arithmetic not supported.");
  }

  Ty const* operator()(VectorTy const& lhs, VectorTy const& rhs) {
    if (lhs.getNumElements() != rhs.getNumElements()) {
      return logError("Number of elements must match");
    }
    if (operator()(lhs.getBasicTy(), rhs.getBasicTy())) {
      return &lhs;
    }
    return nullptr;
  }

  Ty const* operator()(VectorTy const& lhs, BasicTy const& rhs) {
    if (operator()(lhs.getBasicTy(), rhs)) {
      return &lhs;
    }
    return nullptr;
  }

  Ty const* operator()(BasicTy const& lhs, VectorTy const& rhs) {
    return operator()(rhs, lhs);
  }

  template<typename T, typename U>
  Ty const* operator()(T const&, U const&) {
    return logError("Unknown type combination");
  }
};

#endif // PSPAMM_VISITOR_COMMONTYPE_H_
