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
struct SourceLocation;

enum class MutatorKind {
  Unknown,
  ConditionalsBoundaryMutator,
  MathAddMutator,
  NegateMutator,
  RemoveVoidFunctionMutator
};

class Mutator {
public:
  virtual MutationPoint *getMutationPoint(MullModule *module,
                                            MutationPointAddress &address,
                                            llvm::Instruction *instruction,
                                            SourceLocation &sourceLocation) = 0;

  /// FIXME: Rename to 'getUniqueIdentifier'
  virtual std::string uniqueID() = 0;
  virtual std::string uniqueID() const = 0;
  virtual MutatorKind mutatorKind()  { return MutatorKind::Unknown; }

  virtual bool canBeApplied(llvm::Value &V) = 0;
  virtual llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) = 0;
  virtual ~Mutator() = default;
};

}
