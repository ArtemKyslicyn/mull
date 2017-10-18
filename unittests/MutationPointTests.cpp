
#include "Context.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "MutationOperators/MathAddMutationOperator.h"
#include "MutationOperators/MathDivMutationOperator.h"
#include "MutationOperators/MathMulMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/ScalarValueMutationOperator.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

static llvm::Instruction &FunctionInstructionByAddress(Function &F, MutationPointAddress address) {
  printf("ModuleInstructionByAddress: %d %d %d\n",
         address.getFnIndex(),
         address.getBBIndex(),
         address.getIIndex());

  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &NewInstruction = *(std::next(B.begin(), address.getIIndex()));

  return NewInstruction;
}

Instruction *getFirstNamedInstruction(Function &F, const StringRef &Name) {
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction &Instr = *I;

    //printf("Found instruction: %s\n", Instr.getName().str().c_str());

    if (Instr.getName().equals(Name)) {
      return &*I;
    }
  }

  return nullptr;
}

TEST(MutationPoint, SimpleTest_AddOperator_applyMutation) {
  auto ModuleWithTests   = TestModuleFactory.createTesterModule();
  auto ModuleWithTestees = TestModuleFactory.createTesteeModule();

  auto mullModuleWithTests   = make_unique<MullModule>(std::move(ModuleWithTests), "");
  auto mullModuleWithTestees = make_unique<MullModule>(std::move(ModuleWithTestees), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));
  Ctx.addModule(std::move(mullModuleWithTestees));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  Function *Testee = Ctx.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  std::unique_ptr<TargetMachine> targetMachine(
    EngineBuilder().selectTarget(Triple(), "", "",
    SmallVector<std::string, 1>()));
  
  Compiler compiler(*targetMachine.get());

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("count_letters");
  ASSERT_TRUE(mutatedTestee != nullptr);

  Instruction *mutatedInstruction = getFirstNamedInstruction(*mutatedTestee, "add");
  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Sub, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathSubOperator_applyMutation) {
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
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  std::unique_ptr<TargetMachine> targetMachine(EngineBuilder().selectTarget(Triple(),
                                                                            "",
                                                                            "",
                                                                            SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("math_sub");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Add, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathMulOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathMul_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathMulMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  Function *Testee = Ctx.lookupDefinedFunction("math_mul");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  std::unique_ptr<TargetMachine> targetMachine(EngineBuilder().selectTarget(Triple(),
                                                                            "",
                                                                            "",
                                                                            SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("math_mul");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
  *(std::next(ownedMutatedModule->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
  *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
  &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::SDiv, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_MathDivOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_MathDiv_module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathDivMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  Function *Testee = Ctx.lookupDefinedFunction("math_div");
  ASSERT_FALSE(Testee->empty());

  std::vector<MutationPoint *> MutationPoints = Finder.findMutationPoints(Ctx, *Testee);
  ASSERT_EQ(1U, MutationPoints.size());

  MutationPoint *MP = (*(MutationPoints.begin()));
  MutationPointAddress address = MP->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(MP->getOriginalValue()));

  std::string ReplacedInstructionName = MP->getOriginalValue()->getName().str();

  std::unique_ptr<TargetMachine> targetMachine(EngineBuilder().selectTarget(Triple(),
                                                                            "",
                                                                            "",
                                                                            SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());

  auto ownedMutatedModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("math_div");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
  *(std::next(ownedMutatedModule->begin(), address.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
  *(std::next(mutatedFunction.begin(), address.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
  &*(std::next(mutatedBasicBlock.begin(), address.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<BinaryOperator>(mutatedInstruction));
  ASSERT_EQ(Instruction::Mul, mutatedInstruction->getOpcode());
}

TEST(MutationPoint, SimpleTest_NegateConditionOperator_applyMutation) {
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
  ASSERT_TRUE(isa<CmpInst>(MP->getOriginalValue()));

  MutationPointAddress address = MP->getAddress();

  ASSERT_EQ(&FunctionInstructionByAddress(*Testee, address), MP->getOriginalValue());

  auto ownedMutatedTesteeModule = MP->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedTesteeModule->getFunction("max");
  ASSERT_TRUE(mutatedTestee != nullptr);

  auto &mutatedInstruction = FunctionInstructionByAddress(*mutatedTestee,
                                                          MP->getAddress());
  ASSERT_TRUE(CmpInst::classof(&mutatedInstruction));

  auto mutatedCmpInstruction = cast<CmpInst>(&mutatedInstruction);
  ASSERT_EQ(mutatedCmpInstruction->getPredicate(), CmpInst::Predicate::ICMP_SGE);
}

TEST(MutationPoint, SimpleTest_AndOrMutationOperator_applyMutation) {
  auto module = TestModuleFactory.create_SimpleTest_ANDORReplacement_Module();

  auto mullModule = make_unique<MullModule>(std::move(module), "");

  Context ctx;
  ctx.addModule(std::move(mullModule));

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());

  SimpleTestFinder Finder(std::move(mutationOperators));

  {
    Function *test1_testee1_function = ctx.lookupDefinedFunction("testee_AND_operator_2branches");

    std::vector<MutationPoint *> test1_testee1_mutationPoints =
      Finder.findMutationPoints(ctx, *test1_testee1_function);

    ASSERT_EQ(1U, test1_testee1_mutationPoints.size());

    MutationPoint *test1_testee1_mutationPoint1 = (*(test1_testee1_mutationPoints.begin()));

    MutationPointAddress test1_testee1_mutationPoint1_address =
      test1_testee1_mutationPoint1->getAddress();

    ASSERT_TRUE(isa<BranchInst>(test1_testee1_mutationPoint1->getOriginalValue()));

    ASSERT_EQ(&FunctionInstructionByAddress(*test1_testee1_function,
                                            test1_testee1_mutationPoint1_address),
                                            test1_testee1_mutationPoint1->getOriginalValue());

    auto ownedMutatedTesteeModule = test1_testee1_mutationPoint1->cloneModuleAndApplyMutation();

    Function *test1_mutatedTestee1_function = ownedMutatedTesteeModule->getFunction("testee_AND_operator_2branches");
    ASSERT_TRUE(test1_mutatedTestee1_function != nullptr);

    auto &test1_mutatedTestee1_mutatedInstruction =
      FunctionInstructionByAddress(*test1_mutatedTestee1_function,
                                   test1_testee1_mutationPoint1_address);

    ASSERT_TRUE(BranchInst::classof(&test1_mutatedTestee1_mutatedInstruction));

    auto test1_mutatedTestee1_mutatedBranchInstruction = cast<BranchInst>(&test1_mutatedTestee1_mutatedInstruction);
    ASSERT_TRUE(test1_mutatedTestee1_mutatedBranchInstruction != nullptr);
  }
}

TEST(MutationPoint, SimpleTest_ScalarValueMutationOperator_applyMutation) {
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

  MutationPoint *mutationPoint1 = MutationPoints[0];
  MutationPointAddress mutationPointAddress1 = mutationPoint1->getAddress();
  ASSERT_TRUE(isa<StoreInst>(mutationPoint1->getOriginalValue()));

  MutationPoint *mutationPoint2 = MutationPoints[1];
  MutationPointAddress mutationPointAddress2 = mutationPoint2->getAddress();
  ASSERT_TRUE(isa<StoreInst>(mutationPoint2->getOriginalValue()));

  MutationPoint *mutationPoint3 = MutationPoints[2];
  MutationPointAddress mutationPointAddress3 = mutationPoint3->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint3->getOriginalValue()));

  MutationPoint *mutationPoint4 = MutationPoints[3];
  MutationPointAddress mutationPointAddress4 = mutationPoint4->getAddress();
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoint4->getOriginalValue()));

  std::unique_ptr<TargetMachine> targetMachine(EngineBuilder().selectTarget(Triple(),
                                                                            "",
                                                                            "",
                                                                            SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());

  auto ownedMutatedModule = mutationPoint1->cloneModuleAndApplyMutation();

  Function *mutatedTestee = ownedMutatedModule->getFunction("scalar_value");
  ASSERT_TRUE(mutatedTestee != nullptr);

  llvm::Function &mutatedFunction =
    *(std::next(ownedMutatedModule->begin(), mutationPointAddress1.getFnIndex()));
  llvm::BasicBlock &mutatedBasicBlock =
    *(std::next(mutatedFunction.begin(), mutationPointAddress1.getBBIndex()));
  llvm::Instruction *mutatedInstruction =
    &*(std::next(mutatedBasicBlock.begin(), mutationPointAddress1.getIIndex()));

  ASSERT_TRUE(mutatedInstruction != nullptr);

  ASSERT_TRUE(isa<StoreInst>(mutatedInstruction));
}
