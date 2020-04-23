#ifndef PSPAMM_TYPE_H_
#define PSPAMM_TYPE_H_

#include <memory>
#include <unordered_map>
#include <md/type.hpp>

enum class ArTy {
  Int,
  Float
};

using TyBase = md::type<class BasicTy,
                        class VectorTy>;

class Ty : public TyBase {
public:
  virtual bool isVectorTy() const { return false; }
  virtual std::unique_ptr<Ty> clone() const = 0;
};

class BasicTy : public md::with_type<BasicTy,Ty> {
private:
  static const std::unordered_map<ArTy,std::string> ArTy2String;

  ArTy type;
  int64_t numBits;
  bool ptrTy;

public:
  BasicTy(ArTy type, int64_t numBits, bool ptrTy)
    : type(type), numBits(numBits), ptrTy(ptrTy) {
  }
  std::unique_ptr<Ty> clone() const override {
    return std::make_unique<BasicTy>(*this);
  }

  std::string const& getTyName() const { return ArTy2String.at(type); }
  ArTy getArTy() const { return type; }
  int64_t getNumBits() const { return numBits; }
  bool isPtrTy() const { return ptrTy; }
};

class VectorTy : public md::with_type<VectorTy,Ty> {
private:
  BasicTy basic;
  int64_t numElements;

public:
  VectorTy(BasicTy basic, int64_t numElements)
    : basic(basic), numElements(numElements) {
  }
  std::unique_ptr<Ty> clone() const override {
    return std::make_unique<VectorTy>(*this);
  }

  BasicTy const& getBasicTy() const { return basic; }
  int64_t getNumElements() const { return numElements; }
  virtual bool isVectorTy() const { return true; }
};

#endif // PSPAMM_TYPE_H_
