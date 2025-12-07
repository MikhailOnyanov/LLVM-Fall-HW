#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

std::string describeValue(Value *V) {
  if (auto *I = dyn_cast<Instruction>(V)) {
    return I->getOpcodeName();
  }

  if (auto *A = dyn_cast<Argument>(V)) {
    if (A->hasName())
      return ("arg:" + A->getName()).str();
    return "arg";
  }

  if (auto *C = dyn_cast<Constant>(V)) {
    std::string Text;
    raw_string_ostream OS(Text);
    C->printAsOperand(OS, /*PrintType=*/false);
    return OS.str();
  }

  return "value";
}

class AppIRTracePass : public PassInfoMixin<AppIRTracePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

PreservedAnalyses AppIRTracePass::run(Module &M, ModuleAnalysisManager &) {
  LLVMContext &Ctx = M.getContext();
  Type *CharPtrTy = PointerType::get(Ctx, 0);

  FunctionCallee LogInstruction =
      M.getOrInsertFunction("log_instruction",
                            FunctionType::get(Type::getVoidTy(Ctx), {CharPtrTy},
                                              false));
  FunctionCallee LogUse = M.getOrInsertFunction(
      "log_use",
      FunctionType::get(Type::getVoidTy(Ctx), {CharPtrTy, CharPtrTy}, false));

  bool Changed = false;

  for (Function &F : M) {
    if (F.isDeclaration())
      continue;

    for (Instruction &I : instructions(F)) {
      if (isa<PHINode>(I))
        continue; // Skip phi as User

      IRBuilder<> Builder(&I);
      Value *UserStr = Builder.CreateGlobalString(I.getOpcodeName());

      // Instruction execution trace
      Builder.CreateCall(LogInstruction, {UserStr});

      // User <- Operand trace
      for (Value *Op : I.operands()) {
        if (isa<MetadataAsValue>(Op))
          continue;

        if (auto *Phi = dyn_cast<PHINode>(Op)) {
          for (Value *Incoming : Phi->incoming_values()) {
            Value *OperandStr =
                Builder.CreateGlobalString(describeValue(Incoming));
            Builder.CreateCall(LogUse, {UserStr, OperandStr});
          }
          continue;
        }

        Value *OperandStr = Builder.CreateGlobalString(describeValue(Op));
        Builder.CreateCall(LogUse, {UserStr, OperandStr});
      }

      Changed = true;
    }
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // namespace

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AppIRTracePass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerOptimizerLastEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel,
                   ThinOrFullLTOPhase) { MPM.addPass(AppIRTracePass()); });

            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "app-ir-trace") {
                    MPM.addPass(AppIRTracePass());
                    return true;
                  }
                  return false;
                });
          }};
}
