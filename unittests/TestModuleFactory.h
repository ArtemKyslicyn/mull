
#include "ModuleLoader.h"
#include "MullModule.h"

#include "llvm/IR/Module.h"

#include <string>

using namespace llvm;
using namespace mull;

class TestModuleFactory {

public:
  std::string testerModulePath_IR();
  std::string testerModulePath_Bitcode();
  
  std::unique_ptr<Module> createModule(const char *fixtureName, const char *moduleIdentifier);
  
  std::unique_ptr<Module> createTesterModule();
  std::unique_ptr<Module> createTesteeModule();

  std::unique_ptr<Module> create_SimpleTest_NegateCondition_Tester_Module();
  std::unique_ptr<Module> create_SimpleTest_NegateCondition_Testee_Module();

  std::unique_ptr<Module> create_SimpleTest_RemoveVoidFunction_Tester_Module();
  std::unique_ptr<Module> create_SimpleTest_RemoveVoidFunction_Testee_Module();

  std::unique_ptr<Module> create_SimpleTest_ANDORReplacement_Module();
  std::unique_ptr<Module> create_SimpleTest_ANDORReplacement_CPPContent_Module();

  std::unique_ptr<Module> create_SimpleTest_testeePathCalculation_testee();
  std::unique_ptr<Module> create_SimpleTest_testeePathCalculation_tester();

  std::unique_ptr<Module> createLibCTesterModule();
  std::unique_ptr<Module> createLibCTesteeModule();

  std::unique_ptr<Module> createExternalLibTesterModule();
  std::unique_ptr<Module> createExternalLibTesteeModule();

  std::unique_ptr<Module> createGoogleTestTesterModule();
  std::unique_ptr<Module> createGoogleTestTesteeModule();
  std::unique_ptr<Module> createGoogleTestFinder_invokeInstTestee_Module();

  std::unique_ptr<Module> APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutation_operatorModule();
  std::unique_ptr<Module> APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutation_operatorModule();
  std::unique_ptr<Module> APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutation_operatorModule();

#pragma mark - Rust
  std::unique_ptr<Module> rustModule();

};

class FakeModuleLoader : public mull::ModuleLoader {
  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules;

public:
  FakeModuleLoader(LLVMContext &context, std::function<std::vector<std::unique_ptr<MullModule>> ()> modules) : ModuleLoader(context), modules(modules) {}

  std::vector<std::unique_ptr<MullModule>>
  loadModulesFromBitcodeFileList(const std::vector<std::string> &paths) override {
    std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = this->modules;

    if (modules) {
      return modules();
    }

    return {};
  }
  
};
