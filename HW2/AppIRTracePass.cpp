#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
using namespace llvm;

struct AppIRTracePass : public PassInfoMixin<AppIRTracePass> {
    Type *voidType;
    Type *int8PtrTy;
    Type *int32Ty;
    Type *int64Ty;

    bool isFuncLogger(StringRef name) {
        return name == "binOptLogger" || name == "callLogger" ||
               name == "funcStartLogger" || name == "funcEndLogger" ||
               name == "resIntLogger";
    }

    bool insertFuncStartLog(Module &M, Function &F, IRBuilder<> &builder) {
        ArrayRef<Type *> params = {int8PtrTy};
        FunctionType *ftype = FunctionType::get(voidType, params, false);
        FunctionCallee func = M.getOrInsertFunction("funcStartLogger", ftype);

        BasicBlock &entryBB = F.getEntryBlock();
        builder.SetInsertPoint(&entryBB.front());
        Value *funcName = builder.CreateGlobalStringPtr(F.getName());
        builder.CreateCall(func, {funcName});
        return true;
    }

    bool insertCallLog(Module &M, Function &F, IRBuilder<> &builder) {
        ArrayRef<Type *> callParams = {int8PtrTy, int8PtrTy, int64Ty};
        FunctionType *callFT = FunctionType::get(voidType, callParams, false);
        FunctionCallee callFunc = M.getOrInsertFunction("callLogger", callFT);

        ArrayRef<Type *> resParams = {int64Ty, int64Ty};
        FunctionType *resFT = FunctionType::get(voidType, resParams, false);
        FunctionCallee resFunc = M.getOrInsertFunction("resIntLogger", resFT);

        bool inserted = false;

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (isa<PHINode>(&I)) continue; // пропускаем phi

                if (auto *call = dyn_cast<CallInst>(&I)) {
                    Function *callee = call->getCalledFunction();
                    if (callee && !isFuncLogger(callee->getName())) {
                        builder.SetInsertPoint(call);
                        Value *calleeName = builder.CreateGlobalStringPtr(callee->getName());
                        Value *funcName = builder.CreateGlobalStringPtr(F.getName());
                        Value *valID = ConstantInt::get(int64Ty, (int64_t)&I);
                        builder.CreateCall(callFunc, {funcName, calleeName, valID});
                        inserted = true;

                        if (!call->getType()->isVoidTy()) {
                            builder.SetInsertPoint(call->getNextNode());
                            builder.CreateCall(resFunc, {call, valID});
                        }
                    }
                }
            }
        }
        return inserted;
    }

    bool insertFuncEndLog(Module &M, Function &F, IRBuilder<> &builder) {
        ArrayRef<Type *> params = {int8PtrTy, int64Ty};
        FunctionType *ftype = FunctionType::get(voidType, params, false);
        FunctionCallee func = M.getOrInsertFunction("funcEndLogger", ftype);

        bool inserted = false;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (isa<PHINode>(&I)) continue;

                if (auto *ret = dyn_cast<ReturnInst>(&I)) {
                    builder.SetInsertPoint(ret);
                    Value *funcName = builder.CreateGlobalStringPtr(F.getName());
                    Value *valID = ConstantInt::get(int64Ty, (int64_t)&I);
                    builder.CreateCall(func, {funcName, valID});
                    inserted = true;
                }
            }
        }
        return inserted;
    }

    bool insertBinOpLog(Module &M, Function &F, IRBuilder<> &builder) {
        ArrayRef<Type *> params = {int32Ty, int32Ty, int32Ty, int8PtrTy, int8PtrTy, int64Ty};
        FunctionType *ftype = FunctionType::get(voidType, params, false);
        FunctionCallee func = M.getOrInsertFunction("binOptLogger", ftype);

        bool inserted = false;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (isa<PHINode>(&I)) continue;

                if (auto *op = dyn_cast<BinaryOperator>(&I)) {
                    builder.SetInsertPoint(op->getNextNode());
                    Value *lhs = op->getOperand(0);
                    Value *rhs = op->getOperand(1);
                    Value *funcName = builder.CreateGlobalStringPtr(F.getName());
                    Value *opName = builder.CreateGlobalStringPtr(op->getOpcodeName());
                    Value *valID = ConstantInt::get(int64Ty, (int64_t)&I);
                    builder.CreateCall(func, {op, lhs, rhs, opName, funcName, valID});
                    inserted = true;
                }
            }
        }
        return inserted;
    }

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
        LLVMContext &Ctx = M.getContext();
        IRBuilder<> builder(Ctx);
        voidType = Type::getVoidTy(Ctx);
        int8PtrTy = Type::getInt8Ty(Ctx)->getPointerTo();
        int32Ty = Type::getInt32Ty(Ctx);
        int64Ty = Type::getInt64Ty(Ctx);

        for (Function &F : M) {
            if (F.isDeclaration() || isFuncLogger(F.getName()))
                continue;

            insertFuncStartLog(M, F, builder);
            insertCallLog(M, F, builder);
            insertFuncEndLog(M, F, builder);
            insertBinOpLog(M, F, builder);
        }

        return PreservedAnalyses::none();
    }
};

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "AppIRTracePass", "1.0",
        [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback([](ModulePassManager &MPM, auto) {
                MPM.addPass(AppIRTracePass{});
                return true;
            });
        }};
}
