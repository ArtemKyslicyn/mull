
#include "Context.h"
#include "Mutators/MathAddMutator.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "MutationsFinder.h"
#include "Filter.h"
#include "Testee.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestRunner, runTest) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(
                                EngineBuilder().selectTarget(Triple(), "", "",
                                SmallVector<std::string, 1>()));

  Compiler compiler;
  Context Ctx;
  SimpleTestRunner Runner(*targetMachine);
  SimpleTestRunner::ObjectFiles ObjectFiles;
  SimpleTestRunner::OwnedObjectFiles OwnedObjectFiles;

  auto OwnedModuleWithTests   = TestModuleFactory.create_SimpleTest_CountLettersTest_Module();
  auto OwnedModuleWithTestees = TestModuleFactory.create_SimpleTest_CountLetters_Module();

  Module *ModuleWithTests   = OwnedModuleWithTests->getModule();
  Module *ModuleWithTestees = OwnedModuleWithTestees->getModule();

  Ctx.addModule(std::move(OwnedModuleWithTests));
  Ctx.addModule(std::move(OwnedModuleWithTestees));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder mutationsFinder(std::move(mutators), config);
  Filter filter;

  SimpleTestFinder testFinder;

  auto Tests = testFinder.findTests(Ctx, filter);

  ASSERT_NE(0U, Tests.size());

  auto &Test = *(Tests.begin());

  {
    auto Obj = compiler.compileModule(ModuleWithTests, *targetMachine);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  {
    auto Obj = compiler.compileModule(ModuleWithTestees, *targetMachine);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  JITEngine jit;

  /// Here we run test with original testee function
  Runner.loadProgram(ObjectFiles, jit);
  ASSERT_EQ(ExecutionStatus::Passed, Runner.runTest(Test.get(), jit));

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());

  /// afterwards we apply single mutation and run test again
  /// expecting it to fail

  Function *testeeFunction = Ctx.lookupDefinedFunction("count_letters");
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> MutationPoints =
    mutationsFinder.getMutationPoints(Ctx, mergedTestees, filter);

  MutationPoint *MP = (*(MutationPoints.begin()));

  LLVMContext localContext;
  auto ownedMutatedTesteeModule = MP->getOriginalModule()->clone(localContext);
  MP->applyMutation();

  {
    auto Obj = compiler.compileModule(ModuleWithTests, *targetMachine);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  {
    auto Obj = compiler.compileModule(ownedMutatedTesteeModule->getModule(), *targetMachine);
    ObjectFiles.push_back(Obj.getBinary());
    OwnedObjectFiles.push_back(std::move(Obj));
  }

  Runner.loadProgram(ObjectFiles, jit);
  ASSERT_EQ(ExecutionStatus::Failed, Runner.runTest(Test.get(), jit));

  ObjectFiles.erase(ObjectFiles.begin(), ObjectFiles.end());
}
