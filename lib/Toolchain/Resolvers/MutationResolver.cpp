#include "Toolchain/Resolvers/MutationResolver.h"
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>

using namespace mull;
using namespace llvm;

MutationResolver::MutationResolver(orc::LocalCXXRuntimeOverrides &overrides,
                                   std::map<std::string, uint64_t *> &trampolines) :
    overrides(overrides), trampolines(trampolines) {
}

llvm_compat::JITSymbolInfo MutationResolver::findSymbol(const std::string &name) {
/// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return llvm_compat::JITSymbolInfo(address, JITSymbolFlags::Exported);
  }

  auto trampoline = trampolines.find(name);
  if (trampoline != trampolines.end()) {
    return llvm_compat::JITSymbolInfo((uint64_t)trampoline->second, JITSymbolFlags::Exported);
  }

  return llvm_compat::JITSymbolInfo(nullptr);
}

llvm_compat::JITSymbolInfo MutationResolver::findSymbolInLogicalDylib(const std::string &name) {
  return llvm_compat::JITSymbolInfo(nullptr);
}
