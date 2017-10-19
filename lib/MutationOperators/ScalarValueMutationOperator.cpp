#include "MutationOperators/ScalarValueMutationOperator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationOperators/MutationOperatorFilter.h"
#include "MutationPoint.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

enum ScalarValueMutationType {
  None = 1,
  Int,
  Float
};

static
ScalarValueMutationType findPossibleApplication(Value &V,
                                                std::string &outDiagnostics);
static ConstantInt *getReplacementInt(ConstantInt *constantInt);
static ConstantFP *getReplacementFloat(ConstantFP *constantFloat);

const std::string ScalarValueMutationOperator::ID = "scalar_value_mutation_operator";

MutationPoint *
ScalarValueMutationOperator::getMutationPoint(MullModule *module,
                                              MutationPointAddress &address,
                                              llvm::Instruction *instruction) {
  std::string diagnostics;
  ScalarValueMutationType mutationType =
    findPossibleApplication(*instruction, diagnostics);
  if (mutationType == ScalarValueMutationType::None) {
    return nullptr;
  }

  return new MutationPoint(this, address, instruction, module, diagnostics);
}

/// Currently only used by SimpleTestFinder.
bool ScalarValueMutationOperator::canBeApplied(Value &V) {
  std::string diagnostics;
  return findPossibleApplication(V, diagnostics) != ScalarValueMutationType::None;
}

static
ScalarValueMutationType findPossibleApplication(Value &V,
                                                std::string &outDiagnostics) {
  Instruction *instruction = dyn_cast<Instruction>(&V);
  assert(instruction);

  unsigned opcode = instruction->getOpcode();

  if ((opcode >= Instruction::BinaryOpsBegin &&
       opcode < Instruction::BinaryOpsEnd) ||
      opcode == Instruction::Store ||
      opcode == Instruction::FCmp ||
      opcode == Instruction::ICmp ||
      opcode == Instruction::Ret ||
      opcode == Instruction::Call ||
      opcode == Instruction::Invoke) {

    if (CallInst *callInstruction = dyn_cast<CallInst>(instruction)) {
      if (Function *callee = dyn_cast<Function>(callInstruction->getCalledValue())) {
        if (callee->getName().startswith("mull_")) {
          return ScalarValueMutationType::None;
        }
      }
    }

    for (Value *operand: instruction->operands()) {
      if (ConstantInt *constantInt = dyn_cast<ConstantInt>(operand)) {
        auto intValue = constantInt->getValue();

        auto replacementInt = getReplacementInt(constantInt);
        auto replacementIntValue = replacementInt->getValue();

        /// Skip big number because getSExtValue throws otherwise.
        /// TODO: consider these edge cases in unit tests.
        if (intValue.getNumWords() > 1) {
          continue;
        }

        std::stringstream diagstream;
        diagstream << "Scalar Value Replacement: ";

        diagstream << intValue.getSExtValue();
        diagstream << " -> ";
        diagstream << replacementIntValue.getSExtValue();

        outDiagnostics.assign(diagstream.str());

        return ScalarValueMutationType::Int;
      }

      if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(operand)) {
        auto floatValue = constantFloat->getValueAPF();
        auto replacementFloat = getReplacementFloat(constantFloat);
        auto replacementFloatValue = replacementFloat->getValueAPF();

        std::stringstream diagstream;
        diagstream << "Scalar Value Replacement: ";

        diagstream << floatValue.convertToDouble();
        diagstream << " -> ";
        diagstream << replacementFloatValue.convertToDouble();

        outDiagnostics.assign(diagstream.str());

        return ScalarValueMutationType::Float;
      }
    }
  }

  return ScalarValueMutationType::None;
}

static ConstantInt *getReplacementInt(ConstantInt *constantInt) {
  auto type = constantInt->getType();

  auto intValue = constantInt->getValue();

  // TODO: Refactor to avoid unnecessary construction.
  APInt replacementIntValue;

  // TODO: Review the rules for mutation.
  // TODO: Didn't find a better way of testing APInt for zero-ness.
  if (intValue.isNegative() == false && intValue.isStrictlyPositive() == false) {
    replacementIntValue = APInt(8U, 1);
  } else {
    replacementIntValue = APInt(8U, 0);
  }

  ConstantInt *replacement = dyn_cast<ConstantInt>(ConstantInt::get(type, replacementIntValue));

  return replacement;
}

static ConstantFP *getReplacementFloat(ConstantFP *constantFloat) {
  auto floatValue = constantFloat->getValueAPF();

    // TODO: review the rules for mutation.
  if (floatValue.isZero()) {

    // TODO: Didn't find a better way of creating APFloat for number 1.
    APFloat replacementFloatValue = APFloat((double)1);
    ConstantFP *replacement =
      dyn_cast<ConstantFP>(ConstantFP::get(constantFloat->getContext(),
                                           replacementFloatValue));

    return replacement;
  }
  else {
    APFloat replacementFloatValue = APFloat::getZero(floatValue.getSemantics());
    ConstantFP *replacement =
      dyn_cast<ConstantFP>(ConstantFP::get(constantFloat->getContext(),
                                           replacementFloatValue));

    return replacement;
  }

  return nullptr;
}

llvm::Value *
ScalarValueMutationOperator::applyMutation(Module *M,
                                           MutationPointAddress address,
                                           Value &_V) {

  llvm::Instruction &I = address.findInstruction(M);

  for (unsigned int i = 0; i < I.getNumOperands(); i++) {
    Value *operand = I.getOperand(i);

    if (ConstantInt *constantInt = dyn_cast<ConstantInt>(operand)) {
      ConstantInt *replacement = getReplacementInt(constantInt);

      I.setOperand(i, replacement);

      return &I;
    }
    if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(operand)) {
      ConstantFP *replacement = getReplacementFloat(constantFloat);

      I.setOperand(i, replacement);

      return &I;
    }
  }

  llvm_unreachable("Must not reach here!");

  return nullptr;
}
