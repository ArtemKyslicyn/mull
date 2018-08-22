#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vector>

#include "SourceLocation.h"

namespace llvm {

class Function;
class Instruction;
class Value;
class Module;

}

namespace mull {

class Compiler;
class Mutator;
class MullModule;
class Test;

/// \brief Container class that stores information needed to find MutationPoints.
/// We need the indexes of function, basic block and instruction to find the
/// mutation point in the clone of original module, when mutator is
/// to apply mutation in that clone.
class MutationPointAddress {
  int FnIndex;
  int BBIndex;
  int IIndex;

  std::string identifier;
public:
  MutationPointAddress(int FnIndex, int BBIndex, int IIndex) :
    FnIndex(FnIndex), BBIndex(BBIndex), IIndex(IIndex) {

    identifier = std::to_string(FnIndex) + "_" +
      std::to_string(BBIndex) + "_" +
      std::to_string(IIndex);
  }

  int getFnIndex() { return FnIndex; }
  int getBBIndex() { return BBIndex; }
  int getIIndex() { return IIndex; }

  std::string getIdentifier() {
    return identifier;
  }

  std::string getIdentifier() const {
    return identifier;
  }

  llvm::Instruction &findInstruction(llvm::Module *module);
  llvm::Function &findFunction(llvm::Module *module);

  static int getFunctionIndex(llvm::Function *function);
  static
  void enumerateInstructions(llvm::Function &function,
                             const std::function <void (llvm::Instruction &,
                                                        int,
                                                        int)>& block);
};

class MutationPoint {
  Mutator *mutator;
  MutationPointAddress Address;
  llvm::Value *OriginalValue;
  MullModule *module;
  llvm::Function *function;
  std::string uniqueIdentifier;
  std::string diagnostics;
  const SourceLocation sourceLocation;
  std::vector<std::pair<Test *, int>> reachableTests;
public:
  MutationPoint(Mutator *mutator,
                MutationPointAddress Address,
                llvm::Value *Val,
                llvm::Function *function,
                std::string diagnostics,
                const SourceLocation &location,
                MullModule *m);

  ~MutationPoint();

  Mutator *getMutator();
  MutationPointAddress getAddress();
  llvm::Value *getOriginalValue();
  MullModule *getOriginalModule();

  llvm::Function *getFunction();

  Mutator *getMutator() const;
  MutationPointAddress getAddress() const;
  llvm::Value *getOriginalValue() const;
  MullModule *getOriginalModule() const;
  const SourceLocation &getSourceLocation() const;

  void addReachableTest(Test *test, int distance);
  void applyMutation(MullModule &module);

  const std::vector<std::pair<Test *, int>> &getReachableTests() const;

  std::string getUniqueIdentifier();
  std::string getUniqueIdentifier() const;

  const std::string &getDiagnostics();
  const std::string &getDiagnostics() const;
};

}
