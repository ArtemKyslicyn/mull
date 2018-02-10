#pragma once

#include <string>
#include <vector>

namespace llvm {
  class Function;
  class Value;
  class Module;
  class Instruction;
}

namespace mull {

class Context;
class MullModule;
class MutationPoint;
class MutationPointAddress;

enum class MutationOperatorKind {
  Unknown,
  MathAdd,
  NegateCondition
};

class MutationOperator {
public:
  virtual MutationPoint *getMutationPoint(MullModule *module,
                                          MutationPointAddress &address,
                                          llvm::Instruction *instruction) = 0;

  virtual MutationOperatorKind getKind() { return MutationOperatorKind::Unknown; }
  /// FIXME: Renmae to 'getUniqueIdentifier'
  virtual std::string uniqueID() = 0;
  virtual std::string uniqueID() const = 0;

  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) = 0;
  virtual ~MutationOperator() {}
};

}
