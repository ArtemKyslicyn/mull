#include "TestRunner.h"

#include <llvm/Support/DynamicLibrary.h>

using namespace mull;
using namespace llvm;

TestRunner::TestRunner()
{
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}
