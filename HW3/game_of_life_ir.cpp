#include "sim.h"
#include "grid.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

using namespace llvm;

// Глобальные константы
const int GENERATIONS = 2000;
const int GRID_SIZE_VALUE = 64; // Используем значение из grid.h

int main() {
    LLVMContext context;
    Module *module = new Module("game_of_life.c", context);
    IRBuilder<> builder(context);

    // Типы данных
    Type *voidType = Type::getVoidTy(context);
    Type *int32Type = Type::getInt32Ty(context);
    Type *int32PtrType = Type::getInt32Ty(context)->getPointerTo();

    // Объявление внешних функций из sim.h
    std::vector<Type *> simPutPixelParamTypes = {int32Type, int32Type, int32Type};
    FunctionType *simPutPixelType = FunctionType::get(voidType, simPutPixelParamTypes, false);
    FunctionCallee simPutPixelFunc = module->getOrInsertFunction("simPutPixel", simPutPixelType);

    FunctionType *simFlushType = FunctionType::get(voidType, false);
    FunctionCallee simFlushFunc = module->getOrInsertFunction("simFlush", simFlushType);

    FunctionType *simRandType = FunctionType::get(int32Type, false);
    FunctionCallee simRandFunc = module->getOrInsertFunction("simRand", simRandType);

    // Функция renderField (из grid.h)
    std::vector<Type *> renderFieldParamTypes = {int32Type, int32PtrType};
    FunctionType *renderFieldType = FunctionType::get(voidType, renderFieldParamTypes, false);
    FunctionCallee renderFieldFunc = module->getOrInsertFunction("renderField", renderFieldType);

    // Основная функция app()
    FunctionType *appFuncType = FunctionType::get(voidType, false);
    Function *appFunc = Function::Create(appFuncType, Function::ExternalLinkage, "app", module);

    // === ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ===

    // Функция initEmptyField
    std::vector<Type *> initEmptyFieldParamTypes = {int32PtrType, int32Type};
    FunctionType *initEmptyFieldType = FunctionType::get(voidType, initEmptyFieldParamTypes, false);
    Function *initEmptyFieldFunc = Function::Create(initEmptyFieldType, Function::InternalLinkage, "initEmptyField", module);

    BasicBlock *initEmptyFieldBB = BasicBlock::Create(context, "", initEmptyFieldFunc);
    builder.SetInsertPoint(initEmptyFieldBB);
    
    // Параметры функции
    Value *fieldParam = initEmptyFieldFunc->getArg(0);
    Value *sizeParam = initEmptyFieldFunc->getArg(1);
    
    // Цикл инициализации нулями
    BasicBlock *loopBB = BasicBlock::Create(context, "loop", initEmptyFieldFunc);
    BasicBlock *condBB = BasicBlock::Create(context, "cond", initEmptyFieldFunc);
    BasicBlock *exitBB = BasicBlock::Create(context, "exit", initEmptyFieldFunc);
    
    builder.CreateBr(loopBB);
    
    // Loop header
    builder.SetInsertPoint(loopBB);
    PHINode *iPhi = builder.CreatePHI(int32Type, 2, "i");
    iPhi->addIncoming(ConstantInt::get(int32Type, 0), initEmptyFieldBB);
    
    // field[i] = 0
    Value *fieldPtr1 = builder.CreateGEP(int32Type, fieldParam, iPhi);
    builder.CreateStore(ConstantInt::get(int32Type, 0), fieldPtr1);
    
    // i++
    Value *iNext = builder.CreateAdd(iPhi, ConstantInt::get(int32Type, 1), "i_next");
    
    // Условие выхода: i < size*size
    builder.CreateBr(condBB);
    
    builder.SetInsertPoint(condBB);
    Value *maxSize = builder.CreateMul(sizeParam, sizeParam, "max_size");
    Value *cond = builder.CreateICmpSLT(iNext, maxSize, "loop_cond");
    iPhi->addIncoming(iNext, condBB);
    builder.CreateCondBr(cond, loopBB, exitBB);
    
    builder.SetInsertPoint(exitBB);
    builder.CreateRetVoid();

    // Функция getCellValue
    std::vector<Type *> getCellValueParamTypes = {int32PtrType, int32Type, int32Type, int32Type};
    FunctionType *getCellValueType = FunctionType::get(int32Type, getCellValueParamTypes, false);
    Function *getCellValueFunc = Function::Create(getCellValueType, Function::InternalLinkage, "getCellValue", module);

    BasicBlock *getCellValueBB = BasicBlock::Create(context, "", getCellValueFunc);
    builder.SetInsertPoint(getCellValueBB);
    
    Value *fieldParam2 = getCellValueFunc->getArg(0);
    Value *xParam = getCellValueFunc->getArg(1);
    Value *yParam = getCellValueFunc->getArg(2);
    Value *sizeParam2 = getCellValueFunc->getArg(3);
    
    // x = (x + size) % size
    Value *xAdjusted = builder.CreateAdd(xParam, sizeParam2);
    Value *xFinal = builder.CreateSRem(xAdjusted, sizeParam2);
    
    // y = (y + size) % size  
    Value *yAdjusted = builder.CreateAdd(yParam, sizeParam2);
    Value *yFinal = builder.CreateSRem(yAdjusted, sizeParam2);
    
    // field[x * size + y]
    Value *xSize1 = builder.CreateMul(xFinal, sizeParam2);
    Value *index1 = builder.CreateAdd(xSize1, yFinal);
    Value *cellPtr = builder.CreateGEP(int32Type, fieldParam2, index1);
    Value *cellValue = builder.CreateLoad(int32Type, cellPtr);
    
    builder.CreateRet(cellValue);

    // Функция countNeighbours
    std::vector<Type *> countNeighboursParamTypes = {int32Type, int32Type, int32PtrType, int32Type};
    FunctionType *countNeighboursType = FunctionType::get(int32Type, countNeighboursParamTypes, false);
    Function *countNeighboursFunc = Function::Create(countNeighboursType, Function::InternalLinkage, "countNeighbours", module);

    BasicBlock *countNeighboursBB = BasicBlock::Create(context, "", countNeighboursFunc);
    builder.SetInsertPoint(countNeighboursBB);
    
    Value *posX = countNeighboursFunc->getArg(0);
    Value *posY = countNeighboursFunc->getArg(1);
    Value *fieldParam3 = countNeighboursFunc->getArg(2);
    Value *sizeParam3 = countNeighboursFunc->getArg(3);
    
    // int count_neighbours = 0;
    Value *count = ConstantInt::get(int32Type, 0);
    
    // Внешний цикл по dx
    BasicBlock *outerLoopBB = BasicBlock::Create(context, "outer_loop", countNeighboursFunc);
    BasicBlock *outerCondBB = BasicBlock::Create(context, "outer_cond", countNeighboursFunc);
    BasicBlock *innerLoopBB = BasicBlock::Create(context, "inner_loop", countNeighboursFunc);
    BasicBlock *innerCondBB = BasicBlock::Create(context, "inner_cond", countNeighboursFunc);
    BasicBlock *checkCellBB = BasicBlock::Create(context, "check_cell", countNeighboursFunc);
    BasicBlock *incrementBB = BasicBlock::Create(context, "increment", countNeighboursFunc);
    BasicBlock *returnBB = BasicBlock::Create(context, "return", countNeighboursFunc);
    
    builder.CreateBr(outerLoopBB);
    
    // Outer loop: dx = -1
    builder.SetInsertPoint(outerLoopBB);
    PHINode *dxPhi = builder.CreatePHI(int32Type, 2, "dx");
    PHINode *countOuterPhi = builder.CreatePHI(int32Type, 2, "count_outer");
    dxPhi->addIncoming(ConstantInt::get(int32Type, -1), countNeighboursBB);
    countOuterPhi->addIncoming(count, countNeighboursBB);
    
    // cur_x = pos_x + dx
    Value *curX = builder.CreateAdd(posX, dxPhi, "cur_x");
    
    builder.CreateBr(innerLoopBB);
    
    // Inner loop: dy = -1
    builder.SetInsertPoint(innerLoopBB);
    PHINode *dyPhi = builder.CreatePHI(int32Type, 2, "dy");
    PHINode *countInnerPhi = builder.CreatePHI(int32Type, 2, "count_inner");
    dyPhi->addIncoming(ConstantInt::get(int32Type, -1), outerLoopBB);
    countInnerPhi->addIncoming(countOuterPhi, outerLoopBB);
    
    // cur_y = pos_y + dy
    Value *curY = builder.CreateAdd(posY, dyPhi, "cur_y");
    
    // Проверка: if (pos_x == cur_x && pos_y == cur_y) continue
    Value *xEqual = builder.CreateICmpEQ(posX, curX, "x_equal");
    Value *yEqual = builder.CreateICmpEQ(posY, curY, "y_equal");
    Value *bothEqual = builder.CreateAnd(xEqual, yEqual, "both_equal");
    
    builder.CreateCondBr(bothEqual, incrementBB, checkCellBB);
    
    // Проверка клетки
    builder.SetInsertPoint(checkCellBB);
    Value *neighborValue = builder.CreateCall(getCellValueFunc, {fieldParam3, curX, curY, sizeParam3});
    Value *isAlive = builder.CreateICmpEQ(neighborValue, ConstantInt::get(int32Type, 1));
    Value *newCount = builder.CreateSelect(isAlive, 
        builder.CreateAdd(countInnerPhi, ConstantInt::get(int32Type, 1)),
        countInnerPhi);
    
    builder.CreateBr(incrementBB);
    
    // Инкремент dy
    builder.SetInsertPoint(incrementBB);
    PHINode *countAfterCell = builder.CreatePHI(int32Type, 2, "count_after_cell");
    countAfterCell->addIncoming(countInnerPhi, innerLoopBB);
    countAfterCell->addIncoming(newCount, checkCellBB);
    
    Value *dyNext = builder.CreateAdd(dyPhi, ConstantInt::get(int32Type, 1), "dy_next");
    Value *dyCond = builder.CreateICmpSLT(dyNext, ConstantInt::get(int32Type, 2), "dy_cond");
    
    dyPhi->addIncoming(dyNext, incrementBB);
    countInnerPhi->addIncoming(countAfterCell, incrementBB);
    builder.CreateCondBr(dyCond, innerLoopBB, innerCondBB);
    
    // Инкремент dx
    builder.SetInsertPoint(innerCondBB);
    Value *dxNext = builder.CreateAdd(dxPhi, ConstantInt::get(int32Type, 1), "dx_next");
    Value *dxCond = builder.CreateICmpSLT(dxNext, ConstantInt::get(int32Type, 2), "dx_cond");
    
    dxPhi->addIncoming(dxNext, innerCondBB);
    countOuterPhi->addIncoming(countAfterCell, innerCondBB);
    builder.CreateCondBr(dxCond, outerLoopBB, returnBB);
    
    // Возврат результата
    builder.SetInsertPoint(returnBB);
    builder.CreateRet(countAfterCell);

    // Функция stepLife
    std::vector<Type *> stepLifeParamTypes = {int32Type, int32PtrType, int32PtrType};
    FunctionType *stepLifeType = FunctionType::get(voidType, stepLifeParamTypes, false);
    Function *stepLifeFunc = Function::Create(stepLifeType, Function::InternalLinkage, "stepLife", module);

    BasicBlock *stepLifeBB = BasicBlock::Create(context, "", stepLifeFunc);
    builder.SetInsertPoint(stepLifeBB);
    
    Value *sizeParam4 = stepLifeFunc->getArg(0);
    Value *fieldParam4 = stepLifeFunc->getArg(1);
    Value *nextFieldParam = stepLifeFunc->getArg(2);
    
    // Вложенные циклы по y и x
    BasicBlock *yLoopBB = BasicBlock::Create(context, "y_loop", stepLifeFunc);
    BasicBlock *yCondBB = BasicBlock::Create(context, "y_cond", stepLifeFunc);
    BasicBlock *xLoopBB = BasicBlock::Create(context, "x_loop", stepLifeFunc);
    BasicBlock *xCondBB = BasicBlock::Create(context, "x_cond", stepLifeFunc);
    BasicBlock *calcBB = BasicBlock::Create(context, "calc", stepLifeFunc);
    BasicBlock *stepExitBB = BasicBlock::Create(context, "exit", stepLifeFunc);
    
    builder.CreateBr(yLoopBB);
    
    // Цикл по y
    builder.SetInsertPoint(yLoopBB);
    PHINode *yPhi = builder.CreatePHI(int32Type, 2, "y");
    yPhi->addIncoming(ConstantInt::get(int32Type, 0), stepLifeBB);
    
    builder.CreateBr(xLoopBB);
    
    // Цикл по x
    builder.SetInsertPoint(xLoopBB);
    PHINode *xPhi = builder.CreatePHI(int32Type, 2, "x");
    xPhi->addIncoming(ConstantInt::get(int32Type, 0), yLoopBB);
    
    // Вызов countNeighbours
    Value *n = builder.CreateCall(countNeighboursFunc, {xPhi, yPhi, fieldParam4, sizeParam4});
    
    // idx = x * size + y
    Value *xSize2 = builder.CreateMul(xPhi, sizeParam4);
    Value *idx = builder.CreateAdd(xSize2, yPhi);
    
    // field[idx]
    Value *fieldIdxPtr = builder.CreateGEP(int32Type, fieldParam4, idx);
    Value *currentCell = builder.CreateLoad(int32Type, fieldIdxPtr);
    
    // Проверка условий правил игры
    Value *isAliveCell = builder.CreateICmpEQ(currentCell, ConstantInt::get(int32Type, 1));
    
    BasicBlock *aliveBB = BasicBlock::Create(context, "alive", stepLifeFunc);
    BasicBlock *deadBB = BasicBlock::Create(context, "dead", stepLifeFunc);
    BasicBlock *storeBB = BasicBlock::Create(context, "store", stepLifeFunc);
    
    builder.CreateCondBr(isAliveCell, aliveBB, deadBB);
    
    // Клетка жива: nextField[idx] = (n == 2 || n == 3) ? 1 : 0
    builder.SetInsertPoint(aliveBB);
    Value *n2 = builder.CreateICmpEQ(n, ConstantInt::get(int32Type, 2));
    Value *n3 = builder.CreateICmpEQ(n, ConstantInt::get(int32Type, 3));
    Value *aliveCond = builder.CreateOr(n2, n3);
    Value *aliveValue = builder.CreateSelect(aliveCond, 
        ConstantInt::get(int32Type, 1), 
        ConstantInt::get(int32Type, 0));
    builder.CreateBr(storeBB);
    
    // Клетка мертва: nextField[idx] = (n == 3) ? 1 : 0
    builder.SetInsertPoint(deadBB);
    Value *deadCond = builder.CreateICmpEQ(n, ConstantInt::get(int32Type, 3));
    Value *deadValue = builder.CreateSelect(deadCond,
        ConstantInt::get(int32Type, 1),
        ConstantInt::get(int32Type, 0));
    builder.CreateBr(storeBB);
    
    // Сохранение результата
    builder.SetInsertPoint(storeBB);
    PHINode *resultValue = builder.CreatePHI(int32Type, 2, "result");
    resultValue->addIncoming(aliveValue, aliveBB);
    resultValue->addIncoming(deadValue, deadBB);
    
    Value *nextFieldPtr1 = builder.CreateGEP(int32Type, nextFieldParam, idx);
    builder.CreateStore(resultValue, nextFieldPtr1);
    
    // Инкремент x
    Value *xNext = builder.CreateAdd(xPhi, ConstantInt::get(int32Type, 1), "x_next");
    Value *xCond = builder.CreateICmpSLT(xNext, sizeParam4, "x_cond");
    xPhi->addIncoming(xNext, storeBB);
    builder.CreateCondBr(xCond, xLoopBB, xCondBB);
    
    // Инкремент y
    builder.SetInsertPoint(xCondBB);
    Value *yNext = builder.CreateAdd(yPhi, ConstantInt::get(int32Type, 1), "y_next");
    Value *yCond = builder.CreateICmpSLT(yNext, sizeParam4, "y_cond");
    yPhi->addIncoming(yNext, xCondBB);
    builder.CreateCondBr(yCond, yLoopBB, stepExitBB);
    
    builder.SetInsertPoint(stepExitBB);
    builder.CreateRetVoid();

    // Функция randomizeField
    std::vector<Type *> randomizeFieldParamTypes = {int32PtrType, int32Type};
    FunctionType *randomizeFieldType = FunctionType::get(voidType, randomizeFieldParamTypes, false);
    Function *randomizeFieldFunc = Function::Create(randomizeFieldType, Function::InternalLinkage, "randomizeField", module);

    BasicBlock *randomizeBB = BasicBlock::Create(context, "", randomizeFieldFunc);
    builder.SetInsertPoint(randomizeBB);
    
    Value *fieldParam5 = randomizeFieldFunc->getArg(0);
    Value *sizeParam5 = randomizeFieldFunc->getArg(1);
    
    // Вложенные циклы
    BasicBlock *ryLoopBB = BasicBlock::Create(context, "ry_loop", randomizeFieldFunc);
    BasicBlock *ryCondBB = BasicBlock::Create(context, "ry_cond", randomizeFieldFunc);
    BasicBlock *rxLoopBB = BasicBlock::Create(context, "rx_loop", randomizeFieldFunc);
    BasicBlock *rxCondBB = BasicBlock::Create(context, "rx_cond", randomizeFieldFunc);
    BasicBlock *randBB = BasicBlock::Create(context, "rand", randomizeFieldFunc);
    BasicBlock *randExitBB = BasicBlock::Create(context, "exit", randomizeFieldFunc);
    
    builder.CreateBr(ryLoopBB);
    
    builder.SetInsertPoint(ryLoopBB);
    PHINode *ryPhi = builder.CreatePHI(int32Type, 2, "ry");
    ryPhi->addIncoming(ConstantInt::get(int32Type, 0), randomizeBB);
    
    builder.CreateBr(rxLoopBB);
    
    builder.SetInsertPoint(rxLoopBB);
    PHINode *rxPhi = builder.CreatePHI(int32Type, 2, "rx");
    rxPhi->addIncoming(ConstantInt::get(int32Type, 0), ryLoopBB);
    
    // Генерация случайного числа и проверка условия
    Value *randVal = builder.CreateCall(simRandFunc, {});
    Value *randMod = builder.CreateSRem(randVal, ConstantInt::get(int32Type, 5));
    Value *isAliveRand = builder.CreateICmpEQ(randMod, ConstantInt::get(int32Type, 0));
    Value *aliveRand = builder.CreateSelect(isAliveRand,
        ConstantInt::get(int32Type, 1),
        ConstantInt::get(int32Type, 0));
    
    // Сохранение в field[x * size + y]
    Value *rxSize = builder.CreateMul(rxPhi, sizeParam5);
    Value *ridx = builder.CreateAdd(rxSize, ryPhi);
    Value *rfieldPtr = builder.CreateGEP(int32Type, fieldParam5, ridx);
    builder.CreateStore(aliveRand, rfieldPtr);
    
    // Инкремент x
    Value *rxNext = builder.CreateAdd(rxPhi, ConstantInt::get(int32Type, 1), "rx_next");
    Value *rxCond = builder.CreateICmpSLT(rxNext, sizeParam5, "rx_cond");
    rxPhi->addIncoming(rxNext, rxLoopBB);
    builder.CreateCondBr(rxCond, rxLoopBB, rxCondBB);
    
    // Инкремент y
    builder.SetInsertPoint(rxCondBB);
    Value *ryNext = builder.CreateAdd(ryPhi, ConstantInt::get(int32Type, 1), "ry_next");
    Value *ryCond = builder.CreateICmpSLT(ryNext, sizeParam5, "ry_cond");
    ryPhi->addIncoming(ryNext, rxCondBB);
    builder.CreateCondBr(ryCond, ryLoopBB, randExitBB);
    
    builder.SetInsertPoint(randExitBB);
    builder.CreateRetVoid();

    // === ОСНОВНАЯ ФУНКЦИЯ APP() ===
    
    BasicBlock *appBB = BasicBlock::Create(context, "", appFunc);
    builder.SetInsertPoint(appBB);
    
    // const int size = GRID_SIZE;
    Value *sizeVal = ConstantInt::get(int32Type, GRID_SIZE_VALUE);
    
    // Выделение памяти для field и nextField (статические массивы)
    Value *fieldArray = builder.CreateAlloca(ArrayType::get(int32Type, GRID_SIZE_VALUE * GRID_SIZE_VALUE), nullptr, "field");
    Value *nextFieldArray = builder.CreateAlloca(ArrayType::get(int32Type, GRID_SIZE_VALUE * GRID_SIZE_VALUE), nullptr, "nextField");
    
    // Приведение к int32PtrType
    Value *fieldPtr2 = builder.CreateBitCast(fieldArray, int32PtrType);
    Value *nextFieldPtr2 = builder.CreateBitCast(nextFieldArray, int32PtrType);
    
    // initEmptyField(field, size);
    builder.CreateCall(initEmptyFieldFunc, {fieldPtr2, sizeVal});
    
    // randomizeField(field, size);
    builder.CreateCall(randomizeFieldFunc, {fieldPtr2, sizeVal});
    
    // Главный цикл симуляции (gen от 0 до GENERATIONS)
    BasicBlock *genLoopBB = BasicBlock::Create(context, "gen_loop", appFunc);
    BasicBlock *genCondBB = BasicBlock::Create(context, "gen_cond", appFunc);
    BasicBlock *genBodyBB = BasicBlock::Create(context, "gen_body", appFunc);
    BasicBlock *swapBB = BasicBlock::Create(context, "swap", appFunc);
    BasicBlock *appExitBB = BasicBlock::Create(context, "exit", appFunc);
    
    builder.CreateBr(genLoopBB);
    
    builder.SetInsertPoint(genLoopBB);
    PHINode *genPhi = builder.CreatePHI(int32Type, 2, "gen");
    genPhi->addIncoming(ConstantInt::get(int32Type, 0), appBB);
    
    builder.CreateBr(genBodyBB);
    
    // Тело цикла поколений
    builder.SetInsertPoint(genBodyBB);
    
    // renderField(size, field);
    builder.CreateCall(renderFieldFunc, {sizeVal, fieldPtr2});
    
    // stepLife(size, field, nextField);
    builder.CreateCall(stepLifeFunc, {sizeVal, fieldPtr2, nextFieldPtr2});
    
    // Обмен буферов: копирование nextField -> field
    BasicBlock *swapLoopBB = BasicBlock::Create(context, "swap_loop", appFunc);
    BasicBlock *swapCondBB = BasicBlock::Create(context, "swap_cond", appFunc);
    BasicBlock *afterSwapBB = BasicBlock::Create(context, "after_swap", appFunc);
    
    builder.CreateBr(swapLoopBB);
    
    builder.SetInsertPoint(swapLoopBB);
    PHINode *swapPhi = builder.CreatePHI(int32Type, 2, "swap_i");
    swapPhi->addIncoming(ConstantInt::get(int32Type, 0), genBodyBB);
    
    // field[i] = nextField[i]
    Value *nextFieldElemPtr = builder.CreateGEP(int32Type, nextFieldPtr2, swapPhi);
    Value *nextFieldElem = builder.CreateLoad(int32Type, nextFieldElemPtr);
    Value *fieldElemPtr = builder.CreateGEP(int32Type, fieldPtr2, swapPhi);
    builder.CreateStore(nextFieldElem, fieldElemPtr);
    
    // i++
    Value *swapNext = builder.CreateAdd(swapPhi, ConstantInt::get(int32Type, 1), "swap_next");
    Value *maxSwap = builder.CreateMul(sizeVal, sizeVal, "max_swap");
    Value *swapCond = builder.CreateICmpSLT(swapNext, maxSwap, "swap_cond");
    
    swapPhi->addIncoming(swapNext, swapLoopBB);
    builder.CreateCondBr(swapCond, swapLoopBB, afterSwapBB);
    
    // После обмена буферов
    builder.SetInsertPoint(afterSwapBB);
    
    // Инкремент gen
    Value *genNext = builder.CreateAdd(genPhi, ConstantInt::get(int32Type, 1), "gen_next");
    Value *genCond = builder.CreateICmpSLT(genNext, ConstantInt::get(int32Type, GENERATIONS), "gen_cond");
    
    genPhi->addIncoming(genNext, afterSwapBB);
    builder.CreateCondBr(genCond, genLoopBB, genCondBB);
    
    // Финальный рендер
    builder.SetInsertPoint(genCondBB);
    builder.CreateCall(renderFieldFunc, {sizeVal, fieldPtr2});
    builder.CreateRetVoid();

    // === ВЕРИФИКАЦИЯ И ВЫПОЛНЕНИЕ ===

    // Вывод сгенерированного IR
    module->print(outs(), nullptr);
    outs() << '\n';
    
    // Верификация модуля
    bool verif = verifyModule(*module, &outs());
    outs() << "[VERIFICATION] " << (verif ? "FAIL\n\n" : "OK\n\n");

    // Инициализация и выполнение через ExecutionEngine
    outs() << "[EE] Run Game of Life\n";
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    ExecutionEngine *ee = EngineBuilder(std::unique_ptr<Module>(module)).create();
    
    // Проброс графических функций
    ee->InstallLazyFunctionCreator([](const std::string &fnName) -> void * {
        if (fnName == "simPutPixel") {
            return reinterpret_cast<void *>(simPutPixel);
        }
        if (fnName == "simFlush") {
            return reinterpret_cast<void *>(simFlush);
        }
        if (fnName == "simRand") {
            return reinterpret_cast<void *>(simRand);
        }
        if (fnName == "renderField") {
            return reinterpret_cast<void *>(renderField);
        }
        return nullptr;
    });
    
    ee->finalizeObject();

    // Инициализация симуляции
    simInit();

    // Запуск функции app()
    ArrayRef<GenericValue> noargs;
    GenericValue v = ee->runFunction(appFunc, noargs);
    outs() << "[EE] Execution completed\n";

    simExit();
    return EXIT_SUCCESS;
}