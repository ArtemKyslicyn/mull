#include "Config.h"
#include "Context.h"
#include "Mutators/ConditionalsBoundaryMutator.h"
#include "MutationPoint.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "Toolchain/Toolchain.h"
#include "Filter.h"
#include "Testee.h"
#include "MutationsFinder.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(ConditionalsBoundaryMutator, findMutations) {
  auto mullModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto module = mullModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutators));
  Filter filter;

  for (auto &function : *module) {
    Testee testee(&function, nullptr, 1);

    finder.recordMutationPoints(mullContext, testee, filter);
  }

  std::vector<MutationPoint *> allMutationPoints(finder.getAllMutationPoints());

  ASSERT_EQ(allMutationPoints.size(), 7U);
}

TEST(ConditionalsBoundaryMutator, applyMutations) {
  auto mullModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto mutatedModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto borrowedModule = mullModule.get();
  auto module = borrowedModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutators));
  Filter filter;


  for (auto &function : *module) {
    Testee testee(&function, nullptr, 1);

    finder.recordMutationPoints(mullContext, testee, filter);
  }
  std::vector<MutationPoint *> allMutationPoints(finder.getAllMutationPoints());

  for (auto point: allMutationPoints) {
    Instruction *originalInstruction = &point->getAddress().findInstruction(module);
    point->applyMutation(*mutatedModule.get());
    Instruction *mutatedInstruction = &point->getAddress().findInstruction(mutatedModule->getModule());

    if (ConditionalsBoundaryMutator::isGT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGTE(mutatedInstruction));
    }
    else if (ConditionalsBoundaryMutator::isGTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGT(mutatedInstruction));
    }
    else if (ConditionalsBoundaryMutator::isLTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLT(mutatedInstruction));
    }
    else if (ConditionalsBoundaryMutator::isLT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLTE(mutatedInstruction));
    }
  }

  ASSERT_EQ(allMutationPoints.size(), 7U);
}
