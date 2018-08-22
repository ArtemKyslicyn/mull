#pragma once

#include "ExecutionResult.h"
#include "LLVMCompatibility.h"

#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace mull {

class JITEngine;
class Test;
class Instrumentation;

class TestRunner {
protected:
  llvm::TargetMachine &machine;
public:
  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> OwnedObjectFiles;

  explicit TestRunner(llvm::TargetMachine &targetMachine);

  virtual void loadInstrumentedProgram(ObjectFiles &objectFiles, Instrumentation &instrumentation, JITEngine &jit) = 0;
  virtual void loadMutatedProgram(ObjectFiles &objectFiles, std::map<std::string, uint64_t *> &trampolines, JITEngine &jit) = 0;
  virtual ExecutionStatus runTest(Test *test, JITEngine &jit) = 0;

  virtual ~TestRunner() = default;
};

}
