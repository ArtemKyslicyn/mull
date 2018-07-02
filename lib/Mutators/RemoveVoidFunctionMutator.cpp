#include "Mutators/RemoveVoidFunctionMutator.h"

#include "MutationPoint.h"
#include "Context.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string RemoveVoidFunctionMutator::ID =
  "remove_void_function_mutator";

std::string getDiagnostics(Instruction &instruction) {
  assert(isa<CallInst>(instruction));
  
  CallSite callSite = CallSite(&instruction);

  std::stringstream diagstream;

  diagstream << "Remove Void Call: removed ";
  diagstream << callSite.getCalledFunction()->getName().str();

  std::string diagnostics = diagstream.str();

  return diagnostics;
}


MutationPoint *
RemoveVoidFunctionMutator::getMutationPoint(MullModule *module,
                                            MutationPointAddress &address,
                                            llvm::Instruction *instruction,
                                            SourceLocation &sourceLocation) {
  if (canBeApplied(*instruction)) {
    std::string diagnostics = getDiagnostics(*instruction);
    return new MutationPoint(this, address, instruction, module, diagnostics, sourceLocation);
  }

  return nullptr;
}

bool RemoveVoidFunctionMutator::canBeApplied(Value &V) {
  if (CallInst *callInst = dyn_cast<CallInst>(&V)) {

    /// How it can be that there is no a called function?
    if (Function *calledFunction = callInst->getCalledFunction()) {
      if (calledFunction->getName().startswith("llvm.dbg.declare")) {
        return false;
      }

      if (calledFunction->getName().startswith("llvm.dbg.value")) {
        return false;
      }

      if (calledFunction->getName().startswith("mull_")) {
        return false;
      }

      if (calledFunction->getName().str().find("clang_call_terminate") != std::string::npos) {
        return false;
      }

      /// Do not remove 'void operator delete(void *)'
      if (calledFunction->getName().endswith("ZdlPv")) {
        return false;
      }

      /// TODO: This might also filter out important code. Review this later.
      if (calledFunction->getName().endswith("D1Ev") ||
          calledFunction->getName().endswith("D2Ev") ||
          calledFunction->getName().str().find("C1E") != std::string::npos ||
          calledFunction->getName().str().find("C2E") != std::string::npos) {
        return false;
      }

      if (calledFunction->getReturnType()->getTypeID() == Type::VoidTyID) {
        return true;
      }
    }
  }

  return false;
}

llvm::Value *RemoveVoidFunctionMutator::applyMutation(Module *M, MutationPointAddress address, Value &_V) {
  llvm::Function &F    = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B  = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  CallInst *callInst = dyn_cast<CallInst>(&I);
  callInst->eraseFromParent();

  /// return value here is not used and doesn't do anything outside.
  /// TODO: remove?
  return callInst;
}
