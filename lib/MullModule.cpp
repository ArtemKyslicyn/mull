#include "MullModule.h"
#include "Logger.h"
#include "LLVMCompatibility.h"
#include "MutationPoint.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Path.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace mull;
using namespace llvm;
using namespace std;

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule)
  : module(std::move(llvmModule)),
    uniqueIdentifier("")
{
}

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule,
                       const std::string &md5,
                       const std::string &path)
: module(std::move(llvmModule)), modulePath(path)
{
  uniqueIdentifier =
    llvm::sys::path::stem(module->getModuleIdentifier()).str() + "_" + md5;
}

std::unique_ptr<MullModule> MullModule::clone(LLVMContext &context) {
  auto bufferOrError = MemoryBuffer::getFile(modulePath);
  if (!bufferOrError) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return nullptr;
  }

  auto llvmModule = parseBitcodeFile(bufferOrError->get()->getMemBufferRef(), context);
  if (!llvmModule) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return nullptr;
  }

  auto module = make_unique<MullModule>(std::move(llvmModule.get()), "", modulePath);
  return module;
}

llvm::Module *MullModule::getModule() {
  assert(module.get());
  return module.get();
}

llvm::Module *MullModule::getModule() const {
  assert(module.get());
  return module.get();
}

std::string MullModule::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MullModule::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

std::vector<std::string> MullModule::prepareMutations() {
  std::vector<std::string> mutatedFunctionNames;

  for (auto pair : mutationPoints) {
    auto original = pair.first;
    mutatedFunctionNames.push_back(original->getName().str() + "_" + getUniqueIdentifier());
    for (auto point : pair.second) {
      ValueToValueMapTy map;
      auto mutatedFunction = CloneFunction(original, map);
      mutatedFunction->setName(point->getUniqueIdentifier());
      point->setMutatedFunction(mutatedFunction);
    }
    ValueToValueMapTy map;
    auto originalCopy = CloneFunction(original, map);
    originalCopy->setName(original->getName() + "_" + getUniqueIdentifier() + "_original");
    original->deleteBody();

    std::vector<Value *> args;
    for (unsigned i = 0; i < original->getNumOperands(); i++) {
      auto arg = original->getOperand(i);
      args.push_back(arg);
    }

    auto name = original->getName().str() + "_" + getUniqueIdentifier() + "_trampoline";
    auto trampoline = module->getOrInsertGlobal(name, original->getFunctionType()->getPointerTo());
    BasicBlock *block = BasicBlock::Create(module->getContext(), "indirect_call", original);
    auto loadValue = new LoadInst(trampoline, "indirect_function_pointer", block);
    auto callInst = CallInst::Create(loadValue, args, "indirect_function_call", block);
    ReturnInst::Create(module->getContext(), callInst, block);
  }

  return mutatedFunctionNames;
}

void MullModule::addMutation(MutationPoint *point) {
  std::lock_guard<std::mutex> guard(mutex);
  auto function = point->getOriginalFunction();
  mutationPoints[function].push_back(point);
}
