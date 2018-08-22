#pragma once

#include "MutationResult.h"
#include "Toolchain/JITEngine.h"

namespace mull {

class MutationPoint;
class Driver;
class ProcessSandbox;
class TestRunner;
class Config;
class Toolchain;
class Filter;
class progress_counter;

class MutantExecutionTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  MutantExecutionTask(ProcessSandbox &sandbox,
                      TestRunner &runner,
                      Config &config,
                      Filter &filter,
                      std::vector<llvm::object::ObjectFile *> &objectFiles,
                      std::vector<std::string> &mutatedFunctionNames);

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter);
  JITEngine jit;
  ProcessSandbox &sandbox;
  TestRunner &runner;
  Config &config;
  Filter &filter;
  std::vector<llvm::object::ObjectFile *> &objectFiles;
  std::vector<std::string> &mutatedFunctionNames;
};
}
