
#include "MutationPoint.h"
#include "Mutators/Mutator.h"
#include "Mutators/NegateConditionMutator.h"
#include "Context.h"
#include "Filter.h"
#include "MutationsFinder.h"
#include "Testee.h"

#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include "TestModuleFactory.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static LLVMContext Ctx;

TEST(NegateConditionMutator, canBeApplied) {
  LLVMContext context;

  // Get pointers to the constants.
  Value *One = ConstantInt::get(Type::getInt32Ty(context), 1);
  Value *Two = ConstantInt::get(Type::getInt32Ty(context), 2);

  NegateConditionMutator mutator;

  // Create the "if (arg <= 2) goto exitbb"
//  std::unique_ptr<Value> CondInst = make_unique<ICmpInst>(ICmpInst::ICMP_SLE, One, Two, "cond");
  ICmpInst CondInst(ICmpInst::ICMP_SLE, One, Two, "cond");
  EXPECT_EQ(true, mutator.canBeApplied(CondInst));
}

TEST(NegateConditionMutator, canBeApplied_tobool) {
  LLVMContext context;

  Value *One = ConstantInt::get(Type::getInt32Ty(context), 1);
  Value *Two = ConstantInt::get(Type::getInt32Ty(context), 2);

  NegateConditionMutator mutator;

//  std::unique_ptr<Value> CondInst = make_unique<ICmpInst>(ICmpInst::ICMP_SLE, One, Two, "tobool");
  ICmpInst CondInst(ICmpInst::ICMP_SLE, One, Two, "tobool");

  EXPECT_FALSE(mutator.canBeApplied(CondInst));
}

TEST(NegateConditionMutator, canBeApplied_isnull) {
  LLVMContext context;

  Value *One = ConstantInt::get(Type::getInt32Ty(context), 1);
  Value *Two = ConstantInt::get(Type::getInt32Ty(context), 2);

  NegateConditionMutator mutator;

//  std::unique_ptr<Value> CondInst = make_unique<ICmpInst>(ICmpInst::ICMP_SLE, One, Two, "isnull");
  ICmpInst CondInst(ICmpInst::ICMP_SLE, One, Two, "isnull");

  EXPECT_FALSE(mutator.canBeApplied(CondInst));
}

TEST(NegateConditionMutator, negatedCmpInstPredicate) {
  //llvm::CmpInst::Predicate::

  EXPECT_EQ(NegateConditionMutator::negatedCmpInstPredicate(CmpInst::ICMP_SLT), CmpInst::ICMP_SGE);
}

TEST(NegateConditionMutator, getMutationPoints_no_filter) {
  TestModuleFactory factory;
  auto module = factory.APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutatorModule();
  auto llvmModule = module->getModule();
  assert(llvmModule);

  Function *function = llvmModule->getFunction("_ZN4llvm5APInt12tcExtractBitEPKyj");
  assert(function);
  Testee testee(function, nullptr, 1);

  Context context;
  context.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutators));
  Filter filter;

  auto mutationPoints = finder.getMutationPoints(context, testee, filter);
  EXPECT_EQ(1U, mutationPoints.size());
  EXPECT_EQ(0, mutationPoints[0]->getAddress().getFnIndex());
  EXPECT_EQ(0, mutationPoints[0]->getAddress().getBBIndex());
  EXPECT_EQ(15, mutationPoints[0]->getAddress().getIIndex());
}

TEST(NegateConditionMutator, getMutationPoints_filter_to_bool_converion) {
  TestModuleFactory factory;
  auto module = factory.APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutatorModule();
  auto llvmModule = module->getModule();
  assert(llvmModule);

  Function *function = llvmModule->getFunction("_ZNK4llvm7APFloat11isSignalingEv");
  assert(function);
  Testee testee(function, nullptr, 1);

  Context context;
  context.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutators));
  Filter filter;

  auto mutationPoints = finder.getMutationPoints(context, testee, filter);
  EXPECT_EQ(0U, mutationPoints.size());
}

TEST(NegateConditionMutator, getMutationPoints_filter_is_null) {
  TestModuleFactory factory;
  auto module = factory.APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutatorModule();
  auto llvmModule = module->getModule();
  assert(llvmModule);

  Function *function = llvmModule->getFunction("_ZN4llvm7APFloat15freeSignificandEv");
  assert(function);
  Testee testee(function, nullptr, 1);

  Context context;
  context.addModule(std::move(module));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutators));
  Filter filter;

  auto mutationPoints = finder.getMutationPoints(context, testee, filter);
  EXPECT_EQ(0U, mutationPoints.size());
}
