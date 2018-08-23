#include "Parallelization/Tasks/MutantExecutionTask.h"
#include "Parallelization/Progress.h"
#include "Driver.h"
#include "Config.h"
#include "TestRunner.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

MutantExecutionTask::MutantExecutionTask(ProcessSandbox &sandbox,
                                         TestRunner &runner,
                                         Config &config,
                                         Filter &filter,
                                         std::vector<llvm::object::ObjectFile *> &objectFiles,
                                         std::vector<std::string> &mutatedFunctionNames)
    : sandbox(sandbox), runner(runner), config(config),
      filter(filter), objectFiles(objectFiles), mutatedFunctionNames(mutatedFunctionNames) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage, progress_counter &counter) {
  std::map<std::string, uint64_t *> trampolines;
  for (auto &name: mutatedFunctionNames) {
    auto trampolineName = std::string("_") + name + "_trampoline";
    trampolines.insert(std::make_pair(trampolineName, new uint64_t));
  }

  std::string a = std::string("\ntramps: ") + std::to_string(trampolines.size()) + "\n";
  std::string b = std::string("funcs: ") + std::to_string(mutatedFunctionNames.size()) + "\n";

  errs() << a << b;

  runner.loadMutatedProgram(objectFiles, trampolines, jit);

  for (auto &name: mutatedFunctionNames) {
    auto trampolineName = std::string("_") + name + "_trampoline";
    auto originalName = std::string("_") + name + "_original";
    uint64_t *trampoline = trampolines.at(trampolineName);
    *trampoline = llvm_compat::JITSymbolAddress(jit.getSymbol(originalName));
  }

  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto mutationPoint = *it;

    auto name = mutationPoint->getOriginalFunction()->getName().str();
    auto moduleId = mutationPoint->getOriginalModule()->getUniqueIdentifier();
    auto trampolineName = std::string("_") + name + "_" + moduleId + "_trampoline";
    auto mutatedFunctionName = std::string("_") + mutationPoint->getUniqueIdentifier();
    uint64_t *trampoline = trampolines.at(trampolineName);
    uint64_t address = llvm_compat::JITSymbolAddress(jit.getSymbol(mutatedFunctionName));

    auto atLeastOneTestFailed = false;
    for (auto &reachableTest : mutationPoint->getReachableTests()) {
      auto test = reachableTest.first;
      auto distance = reachableTest.second;

      ExecutionResult result;
      if (config.failFastModeEnabled() && atLeastOneTestFailed) {
        result.status = ExecutionStatus::FailFast;
      } else {
        const auto timeout = test->getExecutionResult().runningTime * 10;
        const auto sandboxTimeout = std::max(30LL, timeout);

        result = sandbox.run([&]() {
          *trampoline = address;
          ExecutionStatus status = runner.runTest(test, jit);
          assert(status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
          return status;
        }, sandboxTimeout);

        assert(result.status != ExecutionStatus::Invalid &&
            "Expect to see valid TestResult");

        if (result.status != ExecutionStatus::Passed) {
          atLeastOneTestFailed = true;
        }
      }

      storage.push_back(make_unique<MutationResult>(result, mutationPoint, distance, test));
    }
  }
}
