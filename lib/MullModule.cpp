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

void MullModule::prepareMutation(MutationPoint *point) {
  auto &function = point->getFunction();
  Function *original = &function;
  if (original->isDeclaration()) {
    original = remappedFunctions[original->getName()];
  } else {
    ValueToValueMapTy map;
    auto copy = CloneFunction(original, map);
    remappedFunctions[original->getName()] = copy;
    original->deleteBody();
    original = copy;
  }

  ValueToValueMapTy map;
  auto copy = CloneFunction(original, map);
  copy->setName(point->getUniqueIdentifier());
  copy->setLinkage(GlobalValue::ExternalLinkage);
}

