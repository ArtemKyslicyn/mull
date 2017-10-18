#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "MutationOperators/MathAddMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/ScalarValueMutationOperator.h"

#include "TestModuleFactory.h"
#include "Test.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestFinder, FindTest) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto mullModuleWithTests = make_unique<MullModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutationOperator>());

  SimpleTestFinder finder(std::move(mutationOperators));

  auto tests = finder.findTests(Ctx);

  ASSERT_EQ(1U, tests.size());
}

TEST(SimpleTestFinder, FindMutationPoints_MathAddMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::unique_ptr<MathAddMutationOperator> addMutationOperator = make_unique<MathAddMutationOperator>();
  mutationOperators.emplace_back(std::move(addMutationOperator));

  SimpleTestFinder Finder(std::move(mutationOperators));
  Function *Testee = Ctx.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  /// TODO: Don't know how to compare unique pointer addMutationOperator with
  /// MutationOperator *.
  //ASSERT_EQ(addMutationOperator.get(), MP->getOperator());
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 2);
  ASSERT_TRUE(MPA.getIIndex() == 1);
}

TEST(SimpleTestFinder, FindMutationPoints_MathSubMutationOperator) {
  auto module = TestModuleFactory.create_SimpleTest_MathSub_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathSubMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  Function *Testee = Ctx.lookupDefinedFunction("math_sub");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 6);
}

TEST(SimpleTestFinder, FindMutationPoints_NegateConditionMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));
  Function *Testee = Ctx.lookupDefinedFunction("max");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  /// TODO: Don't know how to compare unique pointer addMutationOperator with
  /// MutationOperator *.
  //ASSERT_EQ(addMutationOperator.get(), MP->getOperator());
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 7);
}

TEST(SimpleTestFinder, FindMutationPoints_RemoteVoidFunctionMutationOperator) {
  auto ModuleWithTests   = TestModuleFactory.create_SimpleTest_RemoveVoidFunction_Tester_Module();
  auto ModuleWithTestees = TestModuleFactory.create_SimpleTest_RemoveVoidFunction_Testee_Module();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  Function *Testee2 = Ctx.lookupDefinedFunction("testee");
  Function *Testee3 = Ctx.lookupDefinedFunction("void_function");

  ASSERT_FALSE(Testee2->empty());
  ASSERT_FALSE(Testee3->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee2);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  /// TODO: Don't know how to compare unique pointer addMutationOperator with
  /// MutationOperator *.
  //ASSERT_EQ(addMutationOperator.get(), MP->getOperator());
  ASSERT_TRUE(isa<CallInst>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 2);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 2);
}

TEST(SimpleTestFinder, findMutationPoints_AndOrReplacementMutationOperator) {
  auto module = TestModuleFactory.create_SimpleTest_ANDORReplacement_Module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context ctx;
  ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  Function *testee_test_and_operator = ctx.lookupDefinedFunction("testee_AND_operator_2branches");

  std::vector<MutationPoint *> MutationPoints =
    Finder.findMutationPoints(ctx, *testee_test_and_operator);

  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *mutationPoint = (*(MutationPoints.begin()));

  MutationPointAddress mutationPointAddress = mutationPoint->getAddress();
  ASSERT_EQ(mutationPointAddress.getFnIndex(), 0);
  ASSERT_EQ(mutationPointAddress.getBBIndex(), 0);
  ASSERT_EQ(mutationPointAddress.getIIndex(), 10);

  Function *testee5_function = ctx.lookupDefinedFunction("testee_compound_AND_then_OR_operator");

  std::vector<MutationPoint *> testee5_mutationPoints =
    Finder.findMutationPoints(ctx, *testee5_function);

  ASSERT_EQ(2U, testee5_mutationPoints.size());

  MutationPoint *testee5_mutationPoint1 = testee5_mutationPoints[0];

  MutationPointAddress testee5_mutationPoint1_address = testee5_mutationPoint1->getAddress();
  ASSERT_EQ(testee5_mutationPoint1_address.getFnIndex(), 5);
  ASSERT_EQ(testee5_mutationPoint1_address.getBBIndex(), 0);
  ASSERT_EQ(testee5_mutationPoint1_address.getIIndex(), 10);

  MutationPoint *testee5_mutationPoint2 = testee5_mutationPoints[1];

  MutationPointAddress testee5_mutationPoint2_address = testee5_mutationPoint2->getAddress();
  ASSERT_EQ(testee5_mutationPoint2_address.getFnIndex(), 5);
  ASSERT_EQ(testee5_mutationPoint2_address.getBBIndex(), 1);
  ASSERT_EQ(testee5_mutationPoint2_address.getIIndex(), 3);
}

TEST(SimpleTestFinder, FindMutationPoints_ScalarValueMutationOperator) {
  auto module = TestModuleFactory.create_SimpleTest_ScalarValue_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<ScalarValueMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  Function *Testee = Ctx.lookupDefinedFunction("scalar_value");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(4U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));

  ASSERT_TRUE(isa<StoreInst>(MP->getOriginalValue()));

  MutationPointAddress MPA = MP->getAddress();
  ASSERT_TRUE(MPA.getFnIndex() == 0);
  ASSERT_TRUE(MPA.getBBIndex() == 0);
  ASSERT_TRUE(MPA.getIIndex() == 4);
}
