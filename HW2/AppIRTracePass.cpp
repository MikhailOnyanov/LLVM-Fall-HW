#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
using namespace llvm;

struct AppIRTracePass : public PassInfoMixin<AppIRTracePass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {

    LLVMContext &Ctx = M.getContext();
    IRBuilder<> builder(Ctx);

    Type *voidTy = Type::getVoidTy(Ctx);
    Type *int8Ty = Type::getInt8Ty(Ctx);
    Type *int8PtrTy = llvm::PointerType::get(int8Ty, 0); // исправлено для LLVM 21


        // Получаем ссылку на функцию traceInstruction
        FunctionCallee traceFunc = M.getOrInsertFunction(
            "traceInstruction", voidTy, int8PtrTy, int8PtrTy
        );

        for (auto &F : M) {
            if (F.isDeclaration()) continue;

            for (auto &B : F) {
                for (auto &I : B) {

                    // Пропускаем PHI-инструкции
                    if (isa<PHINode>(&I)) continue;

                    builder.SetInsertPoint(&I);

                    Value *funcName = builder.CreateGlobalStringPtr(F.getName());
                    Value *instName = builder.CreateGlobalStringPtr(I.getOpcodeName());

                    builder.CreateCall(traceFunc, {funcName, instName});
                }
            }
        }

        return PreservedAnalyses::none();
    }
};

PassPluginLibraryInfo getPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "AppIRTracePass", "0.0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, auto) {
                    MPM.addPass(AppIRTracePass{});
                    return true;
                });
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getPassPluginInfo();
}
