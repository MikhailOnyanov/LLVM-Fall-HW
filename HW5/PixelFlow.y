%{
/* ============================================
   SECTION 1: C/C++ CODE (Headers, Globals, main)
   ============================================ */

#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include <cstdint>
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

#define YYSTYPE void*

extern "C" {
    int yyparse();
    int yylex();
    void yyerror(char *s) {
        std::cerr << "Parse error: " << s << "\n";
    }
    int yywrap(void) { return 1; }
}

LLVMContext context;
IRBuilder<>* builder;
IRBuilder<>* entryBuilder;
Module* module;
Function* curFunc;

FunctionCallee simPutPixelFunc;
FunctionCallee simFlushFunc;

std::map<std::string, Value*> variables;
std::map<std::string, Value*> arrays;
std::map<std::string, Function*> functions;
std::map<std::string, BasicBlock*> labels;

std::map<int, std::pair<BasicBlock*, BasicBlock*>> ifBlocks;
std::map<int, std::pair<BasicBlock*, BasicBlock*>> loopBlocks;
int ifCounter = 0;
int loopCounter = 0;

std::vector<std::string> currentFunctionParams;
std::string currentReturnVar;
std::vector<Value*> currentArgs;

Value* getOrCreateVariable(const char* name) {
    std::string varName(name);
    if (variables.find(varName) != variables.end()) {
        return variables[varName];
    }

    IRBuilder<> tmpBuilder(&curFunc->getEntryBlock(),
                          curFunc->getEntryBlock().begin());
    Value* alloca = tmpBuilder.CreateAlloca(Type::getInt32Ty(context),
                                            nullptr, varName);
    variables[varName] = alloca;
    return alloca;
}

#ifdef WITH_EXECUTION_ENGINE
extern "C" {
    void simPutPixel(int32_t x, int32_t y, int32_t color);
    void simFlush();
    int32_t simRand();
    void simAssert(int32_t result, int32_t expected);
    void simInit();
    void simExit();
}
#endif

int main(int argc, char **argv) {
#ifdef WITH_EXECUTION_ENGINE
    bool useExecutionEngine = false;
    if (argc > 1 && std::string(argv[1]) == "--exec") {
        useExecutionEngine = true;
    }
#else
    bool useExecutionEngine = false;
#endif

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    module = new Module("PixelFlow", context);
    builder = new IRBuilder<>(context);
    entryBuilder = new IRBuilder<>(context);

    Type *voidType = Type::getVoidTy(context);
    Type *int32Type = Type::getInt32Ty(context);

    ArrayRef<Type*> putPixelParams = {int32Type, int32Type, int32Type};
    FunctionType *putPixelType = FunctionType::get(voidType, putPixelParams, false);
    simPutPixelFunc = module->getOrInsertFunction("simPutPixel", putPixelType);

    FunctionType *flushType = FunctionType::get(voidType, false);
    simFlushFunc = module->getOrInsertFunction("simFlush", flushType);

    FunctionType *randType = FunctionType::get(int32Type, false);
    module->getOrInsertFunction("simRand", randType);

    ArrayRef<Type*> assertParams = {int32Type, int32Type};
    FunctionType *assertType = FunctionType::get(voidType, assertParams, false);
    module->getOrInsertFunction("simAssert", assertType);

    yyparse();

    if (!useExecutionEngine) {
        module->print(outs(), nullptr);
    }

    if (verifyModule(*module, &errs())) {
        errs() << "ERROR: Module verification failed\n";
        return 1;
    }

#ifdef WITH_EXECUTION_ENGINE
    if (useExecutionEngine) {
        outs() << "[EE] Starting ExecutionEngine...\n";

        ExecutionEngine *ee = EngineBuilder(std::unique_ptr<Module>(module)).create();
        if (!ee) {
            errs() << "ERROR: Could not create ExecutionEngine\n";
            return 1;
        }

        ee->InstallLazyFunctionCreator([=](const std::string &fnName) -> void * {
            if (fnName == "simFlush") {
                return reinterpret_cast<void *>(simFlush);
            }
            if (fnName == "simPutPixel") {
                return reinterpret_cast<void *>(simPutPixel);
            }
            if (fnName == "simRand") {
                return reinterpret_cast<void *>(simRand);
            }
            if (fnName == "simAssert") {
                return reinterpret_cast<void *>(simAssert);
            }
            return nullptr;
        });
        ee->finalizeObject();

        simInit();

        Function *appFunc = module->getFunction("app");
        if (appFunc == nullptr) {
            errs() << "ERROR: Can't find app() function\n";
            return 1;
        }

        std::vector<GenericValue> noargs;
        ee->runFunction(appFunc, noargs);

        simExit();
        outs() << "[EE] Done\n";
    }
#endif

    return 0;
}

%}

%token NUMBER IDENTIFIER
%token FUN FOR IN ARRAY CANVAS PIXEL SHOW RGB RAND ASSERT IF ELSE
%token DOTDOT TOK_EQ TOK_NE TOK_LE TOK_GE

%%

Parse: Program { YYACCEPT; }

Program: FunctionDecl
       | Program FunctionDecl
;

ParamList: IDENTIFIER {
               currentFunctionParams.clear();
               currentFunctionParams.push_back((char*)$1);
           }
         | ParamList ',' IDENTIFIER {
               currentFunctionParams.push_back((char*)$3);
           }
;

OptParamList: /* empty */ {
                  currentFunctionParams.clear();
              }
            | ParamList
;

OptReturnVar: /* empty */ {
                  currentReturnVar = "";
              }
            | IDENTIFIER {
                  currentReturnVar = (char*)$1;
              }
;

FunctionDecl: FUN IDENTIFIER '(' OptParamList ')' OptReturnVar {
                  const char* funcName = (char*)$2;

                  std::vector<Type*> paramTypes;
                  for (size_t i = 0; i < currentFunctionParams.size(); i++) {
                      paramTypes.push_back(Type::getInt32Ty(context));
                  }

                  Type* returnType = currentReturnVar.empty()
                      ? Type::getVoidTy(context)
                      : Type::getInt32Ty(context);

                  FunctionType *funcType = FunctionType::get(
                      returnType,
                      paramTypes,
                      false
                  );

                  curFunc = Function::Create(
                      funcType,
                      Function::ExternalLinkage,
                      funcName,
                      module
                  );
                  functions[funcName] = curFunc;

                  BasicBlock *entryBB = BasicBlock::Create(context, "entry", curFunc);
                  builder->SetInsertPoint(entryBB);
                  entryBuilder->SetInsertPoint(entryBB);

                  variables.clear();
                  arrays.clear();
                  labels.clear();

                  auto argIt = curFunc->arg_begin();
                  for (const auto& paramName : currentFunctionParams) {
                      Value* alloca = entryBuilder->CreateAlloca(Type::getInt32Ty(context), nullptr, paramName);
                      builder->CreateStore(&*argIt, alloca);
                      variables[paramName] = alloca;
                      ++argIt;
                  }
              }
              Block {
                  if (!builder->GetInsertBlock()->getTerminator()) {
                      if (currentReturnVar.empty()) {
                          builder->CreateRetVoid();
                      } else {
                          Value* var = getOrCreateVariable(currentReturnVar.c_str());
                          Value* retVal = builder->CreateLoad(Type::getInt32Ty(context), var);
                          builder->CreateRet(retVal);
                      }
                  }
              }
;

Block: '{' Statements '}'
;

Statements: /* empty */
          | Statements Statement
;

Statement: Assignment
         | ArrayAssignment
         | ArrayDecl
         | ForLoop
         | FunctionCall ';'
         | IfStatement
;

Assignment: IDENTIFIER '=' Expression ';' {
                const char* varName = (char*)$1;
                Value* var = getOrCreateVariable(varName);
                Value* expr = (Value*)$3;
                builder->CreateStore(expr, var);
            }
;

ArrayDecl: ARRAY IDENTIFIER '[' Expression ']' ';' {
                const char* arrayName = (char*)$2;
                Value* sizeVal = (Value*)$4;

                ConstantInt* sizeConst = dyn_cast<ConstantInt>(sizeVal);
                if (!sizeConst) {
                    std::cerr << "Error: Array size must be a constant\n";
                    YYERROR;
                }

                uint64_t size = sizeConst->getZExtValue();
                ArrayType* arrayType = ArrayType::get(Type::getInt32Ty(context), size);

                IRBuilder<> tmpBuilder(&curFunc->getEntryBlock(), curFunc->getEntryBlock().begin());
                Value* arrayAlloca = tmpBuilder.CreateAlloca(arrayType, nullptr, arrayName);
                arrays[arrayName] = arrayAlloca;

                Value* zeroIdx = builder->getInt32(0);
                for (uint64_t i = 0; i < size; i++) {
                    Value* indices[] = {zeroIdx, builder->getInt32(i)};
                    Value* elemPtr = builder->CreateGEP(arrayType, arrayAlloca, indices);
                    builder->CreateStore(builder->getInt32(0), elemPtr);
                }
            }
;

ArrayAssignment: IDENTIFIER '[' Expression ']' '=' Expression ';' {
                const char* arrayName = (char*)$1;
                if (arrays.find(arrayName) == arrays.end()) {
                    std::cerr << "Error: Array " << arrayName << " not declared\n";
                    YYERROR;
                }

                Value* arrayPtr = arrays[arrayName];
                Value* index = (Value*)$3;
                Value* value = (Value*)$6;

                AllocaInst* allocaInst = cast<AllocaInst>(arrayPtr);
                ArrayType* arrayType = cast<ArrayType>(allocaInst->getAllocatedType());
                Value* indices[] = {builder->getInt32(0), index};
                Value* elemPtr = builder->CreateGEP(arrayType, arrayPtr, indices);
                builder->CreateStore(value, elemPtr);
            }
;

ForLoop: FOR IDENTIFIER IN Expression DOTDOT Expression {
             const char* varName = (char*)$2;
             Value* var = getOrCreateVariable(varName);
             Value* startVal = (Value*)$4;
             Value* endVal = (Value*)$6;

             builder->CreateStore(startVal, var);

             BasicBlock *loopCondBB = BasicBlock::Create(context, "loop.cond", curFunc);
             BasicBlock *loopBodyBB = BasicBlock::Create(context, "loop.body", curFunc);
             BasicBlock *loopEndBB = BasicBlock::Create(context, "loop.end", curFunc);

             builder->CreateBr(loopCondBB);
             builder->SetInsertPoint(loopCondBB);
             Value* i = builder->CreateLoad(Type::getInt32Ty(context), var);
             Value* cond = builder->CreateICmpSLT(i, endVal);
             builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
             builder->SetInsertPoint(loopBodyBB);

             int id = loopCounter++;
             loopBlocks[id] = {loopCondBB, loopEndBB};
             $$ = (void*)(uintptr_t)id;
         }
         Block {
             const char* varName = (char*)$2;
             Value* var = getOrCreateVariable(varName);
             Value* i = builder->CreateLoad(Type::getInt32Ty(context), var);
             Value* nextI = builder->CreateAdd(i, builder->getInt32(1));
             builder->CreateStore(nextI, var);

             int id = (int)(uintptr_t)$7;
             auto& blocks = loopBlocks[id];
             builder->CreateBr(blocks.first);
             builder->SetInsertPoint(blocks.second);
             loopBlocks.erase(id);
         }
;

IfStatement: IF '(' Expression ')' {
                 Value* cond = (Value*)$3;
                 Value* condBool = builder->CreateICmpNE(cond, builder->getInt32(0));

                 BasicBlock *thenBB = BasicBlock::Create(context, "if.then", curFunc);
                 BasicBlock *elseBB = BasicBlock::Create(context, "if.else", curFunc);
                 BasicBlock *endBB = BasicBlock::Create(context, "if.end", curFunc);

                 builder->CreateCondBr(condBool, thenBB, elseBB);
                 builder->SetInsertPoint(thenBB);

                 int id = ifCounter++;
                 ifBlocks[id] = {elseBB, endBB};
                 $$ = (void*)(uintptr_t)id;
             }
             Block {
                 int id = (int)(uintptr_t)$5;
                 auto& blocks = ifBlocks[id];
                 builder->CreateBr(blocks.second);
                 builder->SetInsertPoint(blocks.first);
             }
             ElsePart {
                 int id = (int)(uintptr_t)$5;
                 auto& blocks = ifBlocks[id];
                 builder->CreateBr(blocks.second);
                 builder->SetInsertPoint(blocks.second);
                 ifBlocks.erase(id);
             }
;

ElsePart: /* empty */
        | ELSE Block
;

FunctionCall: PIXEL '(' Expression ',' Expression ',' Expression ')' {
                  Value* x = (Value*)$3;
                  Value* y = (Value*)$5;
                  Value* color = (Value*)$7;
                  Value* args[] = {x, y, color};
                  builder->CreateCall(simPutPixelFunc, args);
              }
            | SHOW '(' ')' {
                  builder->CreateCall(simFlushFunc);
              }
            | ASSERT '(' Expression ',' Expression ')' {
                  Value* result = (Value*)$3;
                  Value* expected = (Value*)$5;
                  Value* args[] = {result, expected};
                  FunctionCallee assertFunc = module->getFunction("simAssert");
                  builder->CreateCall(assertFunc, args);
              }
            | RGB '(' Expression ',' Expression ',' Expression ')' {
                  Value* r = (Value*)$3;
                  Value* g = (Value*)$5;
                  Value* b = (Value*)$7;

                  Value* rShift = builder->CreateShl(r, builder->getInt32(16));
                  Value* gShift = builder->CreateShl(g, builder->getInt32(8));
                  Value* rb = builder->CreateOr(rShift, gShift);
                  $$ = builder->CreateOr(rb, b);
              }
            | IDENTIFIER '(' OptArgList ')' {
                  const char* funcName = (char*)$1;
                  Function* func = module->getFunction(funcName);
                  if (func == nullptr) {
                      std::cerr << "Error: Function " << funcName << " not found\n";
                      YYERROR;
                  }
                  builder->CreateCall(func, currentArgs);
              }
;

Expression: Term
          | Expression '+' Term {
                $$ = builder->CreateAdd((Value*)$1, (Value*)$3);
            }
          | Expression '-' Term {
                $$ = builder->CreateSub((Value*)$1, (Value*)$3);
            }
          | Expression '<' Term {
                $$ = builder->CreateZExt(
                    builder->CreateICmpSLT((Value*)$1, (Value*)$3),
                    Type::getInt32Ty(context)
                );
            }
          | Expression '>' Term {
                $$ = builder->CreateZExt(
                    builder->CreateICmpSGT((Value*)$1, (Value*)$3),
                    Type::getInt32Ty(context)
                );
            }
          | Expression TOK_LE Term {
                $$ = builder->CreateZExt(
                    builder->CreateICmpSLE((Value*)$1, (Value*)$3),
                    Type::getInt32Ty(context)
                );
            }
          | Expression TOK_GE Term {
                $$ = builder->CreateZExt(
                    builder->CreateICmpSGE((Value*)$1, (Value*)$3),
                    Type::getInt32Ty(context)
                );
            }
          | Expression TOK_EQ Term {
                $$ = builder->CreateZExt(
                    builder->CreateICmpEQ((Value*)$1, (Value*)$3),
                    Type::getInt32Ty(context)
                );
            }
          | Expression TOK_NE Term {
                $$ = builder->CreateZExt(
                    builder->CreateICmpNE((Value*)$1, (Value*)$3),
                    Type::getInt32Ty(context)
                );
            }
;

Term: Factor
    | Term '*' Factor {
          $$ = builder->CreateMul((Value*)$1, (Value*)$3);
      }
    | Term '/' Factor {
          $$ = builder->CreateSDiv((Value*)$1, (Value*)$3);
      }
    | Term '%' Factor {
          $$ = builder->CreateSRem((Value*)$1, (Value*)$3);
      }
;

ArgList: Expression {
             currentArgs.clear();
             currentArgs.push_back((Value*)$1);
         }
       | ArgList ',' Expression {
             currentArgs.push_back((Value*)$3);
         }
;

OptArgList: /* empty */ {
                currentArgs.clear();
            }
          | ArgList
;

Factor: NUMBER {
            int value = atoi((char*)$1);
            $$ = builder->getInt32(value);
        }
      | IDENTIFIER {
            const char* varName = (char*)$1;
            Value* var = getOrCreateVariable(varName);
            $$ = builder->CreateLoad(Type::getInt32Ty(context), var);
        }
      | IDENTIFIER '[' Expression ']' {
            const char* arrayName = (char*)$1;
            if (arrays.find(arrayName) == arrays.end()) {
                std::cerr << "Error: Array " << arrayName << " not declared\n";
                YYERROR;
            }

            Value* arrayPtr = arrays[arrayName];
            Value* index = (Value*)$3;

            AllocaInst* allocaInst = cast<AllocaInst>(arrayPtr);
            ArrayType* arrayType = cast<ArrayType>(allocaInst->getAllocatedType());
            Value* indices[] = {builder->getInt32(0), index};
            Value* elemPtr = builder->CreateGEP(arrayType, arrayPtr, indices);
            $$ = builder->CreateLoad(Type::getInt32Ty(context), elemPtr);
        }
      | '(' Expression ')' {
            $$ = $2;
        }
      | '-' Factor {
            $$ = builder->CreateNeg((Value*)$2);
        }
      | RAND '(' ')' {
            FunctionCallee randFunc = module->getFunction("simRand");
            $$ = builder->CreateCall(randFunc);
        }
      | IDENTIFIER '(' OptArgList ')' {
            const char* funcName = (char*)$1;
            Function* func = module->getFunction(funcName);
            if (func == nullptr) {
                std::cerr << "Error: Function " << funcName << " not found\n";
                YYERROR;
            }
            if (func->getReturnType()->isVoidTy()) {
                std::cerr << "Error: Cannot use void function " << funcName << " in expression\n";
                YYERROR;
            }
            $$ = builder->CreateCall(func, currentArgs);
        }
;

%%

/* ============================================
   SECTION 3: EXTRA C++ CODE
   ============================================ */
