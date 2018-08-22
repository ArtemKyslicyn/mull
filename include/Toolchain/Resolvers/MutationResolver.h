#pragma once

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include "LLVMCompatibility.h"

namespace mull {
class MutationResolver : public llvm_compat::SymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
  std::map<std::string, uint64_t *> &trampolines;
public:
  MutationResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides, std::map<std::string, uint64_t *> &trampolines);
  llvm_compat::JITSymbolInfo findSymbol(const std::string &name) override;
  llvm_compat::JITSymbolInfo findSymbolInLogicalDylib(const std::string &name) override;
};
}
