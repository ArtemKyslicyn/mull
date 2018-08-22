#pragma once

#include "TestRunner.h"
#include "Mangler.h"
#include "Toolchain/JITEngine.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace llvm {

class Function;
class Module;

}

namespace mull {
class Instrumentation;
struct InstrumentationInfo;

class CustomTestRunner : public TestRunner {
  Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;
public:

  CustomTestRunner(llvm::TargetMachine &machine);
  ~CustomTestRunner();

  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation,
                               JITEngine &jit) override;
  void loadMutatedProgram(ObjectFiles &objectFiles, std::map<std::string, uint64_t *> &trampolines, JITEngine &jit) override;
  ExecutionStatus runTest(Test *test, JITEngine &jit) override;

private:
  void *getConstructorPointer(const llvm::Function &function, JITEngine &jit);
  void *getFunctionPointer(const std::string &functionName, JITEngine &jit);
  void runStaticConstructor(llvm::Function *function, JITEngine &jit);
};

}
