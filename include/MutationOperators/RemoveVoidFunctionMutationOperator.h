#pragma once

#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/Instructions.h"

#include <vector>

namespace llvm {
  class Instruction;
}

namespace mull {

  class MullModule;
  class MutationPoint;
  class MutationPointAddress;

  class RemoveVoidFunctionMutationOperator : public MutationOperator {
    
  public:
    static const std::string ID;

    MutationPoint *getMutationPoint(MullModule *module,
                                    MutationPointAddress &address,
                                    llvm::Instruction *instruction) override;

    MutationOperatorKind getKind() override {
      return MutationOperatorKind::RemoveVoidFunctionCall;
    }

    std::string uniqueID() override {
      return ID;
    }
    std::string uniqueID() const override {
      return ID;
    }

    bool canBeApplied(llvm::Value &V) override;
    llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
  };
}
