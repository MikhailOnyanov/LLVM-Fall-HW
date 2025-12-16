#include "../HW1/sim.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace llvm;

static const int REG_FILE_SIZE = 16;
static const int GRID_SIZE_VAL = 128;
static const int CELL_COUNT = GRID_SIZE_VAL * GRID_SIZE_VAL;
static int32_t REG_FILE_EXTERNAL[REG_FILE_SIZE];

// Stubs to avoid SDL crashes in MCJIT run (we only need functional semantics).
static void stubPutPixel(int, int, int) {}
static void stubFlush() {}
static int stubRand() { return std::rand(); }

static bool is_label(const std::string &name) {
  return !name.empty() && name.back() == ':';
}

// Helpers to generate instruction bodies once (as IR) and reuse via calls.
struct InstrFns {
  FunctionCallee xorFn, mulFn, subiFn, incFn, putFn, flushFn;
  FunctionCallee allocFn, clearFn, copyFn, getFn, setFn, cntFn, ruleFn, randFn,
      drawFn, selFn, brDummyFn;
};

static Value *loadReg(IRBuilder<> &b, ArrayType *regTy, GlobalVariable *regFile,
                      Value *idx) {
  Value *ptr = b.CreateInBoundsGEP(regTy, regFile,
                                   {b.getInt32(0), idx}, "reg.ptr");
  return b.CreateLoad(b.getInt32Ty(), ptr, "reg");
}

static void storeReg(IRBuilder<> &b, ArrayType *regTy,
                     GlobalVariable *regFile, Value *val, Value *idx) {
  Value *ptr = b.CreateInBoundsGEP(regTy, regFile,
                                   {b.getInt32(0), idx}, "reg.ptr");
  b.CreateStore(val, ptr);
}

static Value *selectArrayPtr(IRBuilder<> &b, GlobalVariable *cur,
                             GlobalVariable *nxt, Value *base) {
  auto *arrTy = cast<ArrayType>(cur->getValueType());
  Value *curPtr = b.CreateInBoundsGEP(arrTy, cur, {b.getInt32(0), b.getInt32(0)},
                                      "cur.ptr");
  Value *nxtPtr = b.CreateInBoundsGEP(arrTy, nxt, {b.getInt32(0), b.getInt32(0)},
                                      "nxt.ptr");
  Value *isCur = b.CreateICmpEQ(base, b.getInt32(0));
  return b.CreateSelect(isCur, curPtr, nxtPtr, "arr.ptr");
}

static FunctionCallee createVoidFn(Module *m, StringRef name,
                                   ArrayRef<Type *> params) {
  auto *ty = FunctionType::get(Type::getVoidTy(m->getContext()), params, false);
  return m->getOrInsertFunction(name, ty);
}

static void buildInstrFns(Module *m, InstrFns &fns, ArrayType *regTy,
                          GlobalVariable *regFile, GlobalVariable *curArr,
                          GlobalVariable *nxtArr) {
  LLVMContext &ctx = m->getContext();
  Type *i32 = Type::getInt32Ty(ctx);
  IRBuilder<> b(ctx);

  auto define = [&](FunctionCallee callee, auto bodyBuilder) {
    Function *fn = cast<Function>(callee.getCallee());
    if (!fn->empty())
      return;
    BasicBlock *bb = BasicBlock::Create(ctx, "entry", fn);
    b.SetInsertPoint(bb);
    bodyBuilder(fn, b);
    if (!bb->getTerminator())
      b.CreateRetVoid();
    verifyFunction(*fn);
  };

  // XOR
  fns.xorFn = createVoidFn(m, "ir_XOR", {i32, i32, i32});
  define(fns.xorFn, [&](Function *fn, IRBuilder<> &B) {
    auto args = fn->args().begin();
    Value *rd = args++;
    Value *ra = args++;
    Value *rb = args++;
    Value *va = loadReg(B, regTy, regFile, ra);
    Value *vb = loadReg(B, regTy, regFile, rb);
    Value *v = B.CreateXor(va, vb);
    storeReg(B, regTy, regFile, v, rd);
    B.CreateRetVoid();
  });

  // MUL
  fns.mulFn = createVoidFn(m, "ir_MUL", {i32, i32, i32});
  define(fns.mulFn, [&](Function *fn, IRBuilder<> &B) {
    auto args = fn->args().begin();
    Value *rd = args++;
    Value *ra = args++;
    Value *rb = args++;
    Value *va = loadReg(B, regTy, regFile, ra);
    Value *vb = loadReg(B, regTy, regFile, rb);
    Value *v = B.CreateMul(va, vb);
    storeReg(B, regTy, regFile, v, rd);
  });

  // SUBi
  fns.subiFn = createVoidFn(m, "ir_SUBi", {i32, i32, i32});
  define(fns.subiFn, [&](Function *fn, IRBuilder<> &B) {
    auto args = fn->args().begin();
    Value *rd = args++;
    Value *ra = args++;
    Value *imm = args++;
    Value *va = loadReg(B, regTy, regFile, ra);
    Value *v = B.CreateSub(va, imm);
    storeReg(B, regTy, regFile, v, rd);
  });

  // INC_NEi
  fns.incFn = createVoidFn(m, "ir_INC_NEi", {i32, i32, i32});
  define(fns.incFn, [&](Function *fn, IRBuilder<> &B) {
    auto args = fn->args().begin();
    Value *rd = args++;
    Value *ra = args++;
    Value *imm = args++;
    Value *va = loadReg(B, regTy, regFile, ra);
    Value *vaInc = B.CreateAdd(va, B.getInt32(1));
    storeReg(B, regTy, regFile, vaInc, ra);
    Value *cmp = B.CreateICmpNE(vaInc, imm);
    Value *ext = B.CreateZExt(cmp, B.getInt32Ty());
    storeReg(B, regTy, regFile, ext, rd);
  });

  // PUT_PIXEL
  fns.putFn = createVoidFn(m, "ir_PUT_PIXEL", {i32, i32, i32});
  define(fns.putFn, [&](Function *fn, IRBuilder<> &B) {
    auto args = fn->args().begin();
    Value *rx = loadReg(B, regTy, regFile, args++);
    Value *ry = loadReg(B, regTy, regFile, args++);
    Value *rc = loadReg(B, regTy, regFile, args++);
    FunctionCallee simPut = m->getOrInsertFunction(
        "simPutPixel", FunctionType::get(Type::getVoidTy(ctx),
                                         {i32, i32, i32}, false));
    B.CreateCall(simPut, {rx, ry, rc});
  });

  // FLUSH
  fns.flushFn = createVoidFn(m, "ir_FLUSH", {});
  define(fns.flushFn, [&](Function *fn, IRBuilder<> &B) {
    FunctionCallee flush = m->getOrInsertFunction(
        "simFlush", FunctionType::get(Type::getVoidTy(ctx), {}, false));
    B.CreateCall(flush);
  });

  // ALLOC_ARRAYS
  fns.allocFn = createVoidFn(m, "ir_ALLOC_ARRAYS", {i32, i32});
  define(fns.allocFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *rcur = it++;
    Value *rnxt = it++;
    storeReg(B, regTy, regFile, B.getInt32(0), rcur);
    storeReg(B, regTy, regFile, B.getInt32(1), rnxt);
  });

  // CLEAR_ARRAY
  fns.clearFn = createVoidFn(m, "ir_CLEAR_ARRAY", {i32, i32});
  define(fns.clearFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *baseIdx = it++;
    Value *len = it++;
    Value *baseVal = loadReg(B, regTy, regFile, baseIdx);
    Value *arrPtr = selectArrayPtr(B, curArr, nxtArr, baseVal);
    BasicBlock *loopBB = BasicBlock::Create(ctx, "clear.loop", fn);
    BasicBlock *bodyBB = BasicBlock::Create(ctx, "clear.body", fn);
    BasicBlock *afterBB = BasicBlock::Create(ctx, "clear.after", fn);
    BasicBlock *pre = B.GetInsertBlock();
    B.CreateBr(loopBB);
    B.SetInsertPoint(loopBB);
    PHINode *i = B.CreatePHI(i32, 2, "i");
    i->addIncoming(B.getInt32(0), pre);
    Value *cmp = B.CreateICmpSLT(i, len);
    B.CreateCondBr(cmp, bodyBB, afterBB);

    B.SetInsertPoint(bodyBB);
    Value *ptr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), arrPtr, i);
    B.CreateStore(B.getInt32(0), ptr);
    Value *incr = B.CreateAdd(i, B.getInt32(1));
    i->addIncoming(incr, bodyBB);
    B.CreateBr(loopBB);

    B.SetInsertPoint(afterBB);
  });

  // COPY_ARRAY
  fns.copyFn = createVoidFn(m, "ir_COPY_ARRAY", {i32, i32, i32});
  define(fns.copyFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *dstIdx = it++;
    Value *srcIdx = it++;
    Value *len = it++;
    Value *dstArr = selectArrayPtr(B, curArr, nxtArr, loadReg(B, regTy, regFile, dstIdx));
    Value *srcArr = selectArrayPtr(B, curArr, nxtArr, loadReg(B, regTy, regFile, srcIdx));
    BasicBlock *loopBB = BasicBlock::Create(ctx, "copy.loop", fn);
    BasicBlock *bodyBB = BasicBlock::Create(ctx, "copy.body", fn);
    BasicBlock *afterBB = BasicBlock::Create(ctx, "copy.after", fn);
    BasicBlock *pre = B.GetInsertBlock();
    B.CreateBr(loopBB);
    B.SetInsertPoint(loopBB);
    PHINode *i = B.CreatePHI(i32, 2, "i");
    i->addIncoming(B.getInt32(0), pre);
    Value *cmp = B.CreateICmpSLT(i, len);
    B.CreateCondBr(cmp, bodyBB, afterBB);

    B.SetInsertPoint(bodyBB);
    Value *sPtr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), srcArr, i);
    Value *dPtr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), dstArr, i);
    Value *val = B.CreateLoad(i32, sPtr);
    B.CreateStore(val, dPtr);
    Value *incr = B.CreateAdd(i, B.getInt32(1));
    i->addIncoming(incr, bodyBB);
    B.CreateBr(loopBB);

    B.SetInsertPoint(afterBB);
  });

  // GET_CELL
  fns.getFn = createVoidFn(m, "ir_GET_CELL", {i32, i32, i32, i32});
  define(fns.getFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *dst = it++;
    Value *base = it++;
    Value *rx = it++;
    Value *ry = it++;
    Value *baseVal = loadReg(B, regTy, regFile, base);
    Value *arr = selectArrayPtr(B, curArr, nxtArr, baseVal);
    Value *xv = loadReg(B, regTy, regFile, rx);
    Value *yv = loadReg(B, regTy, regFile, ry);
    Value *idx = B.CreateAdd(B.CreateMul(xv, B.getInt32(GRID_SIZE_VAL)), yv);
    Value *ptr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), arr, idx);
    Value *val = B.CreateLoad(i32, ptr);
    storeReg(B, regTy, regFile, val, dst);
  });

  // SET_CELL
  fns.setFn = createVoidFn(m, "ir_SET_CELL", {i32, i32, i32, i32});
  define(fns.setFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *base = it++;
    Value *rx = it++;
    Value *ry = it++;
    Value *valReg = it++;
    Value *baseVal = loadReg(B, regTy, regFile, base);
    Value *arr = selectArrayPtr(B, curArr, nxtArr, baseVal);
    Value *xv = loadReg(B, regTy, regFile, rx);
    Value *yv = loadReg(B, regTy, regFile, ry);
    Value *idx = B.CreateAdd(B.CreateMul(xv, B.getInt32(GRID_SIZE_VAL)), yv);
    Value *ptr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), arr, idx);
    Value *val = loadReg(B, regTy, regFile, valReg);
    B.CreateStore(val, ptr);
  });

  // COUNT_NEIGHBORS
  fns.cntFn = createVoidFn(m, "ir_COUNT_NEIGHBORS", {i32, i32, i32, i32});
  define(fns.cntFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *dst = it++;
    Value *base = it++;
    Value *rx = it++;
    Value *ry = it++;
    Value *baseVal = loadReg(B, regTy, regFile, base);
    Value *arr = selectArrayPtr(B, curArr, nxtArr, baseVal);
    Value *xv = loadReg(B, regTy, regFile, rx);
    Value *yv = loadReg(B, regTy, regFile, ry);
    Value *acc = B.getInt32(0);
    const int offs[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                            {0, 1},  {1, -1}, {1, 0},  {1, 1}};
    for (auto &d : offs) {
      Value *cx = B.CreateSRem(
          B.CreateAdd(B.CreateAdd(xv, B.getInt32(d[0])),
                      B.getInt32(GRID_SIZE_VAL)),
          B.getInt32(GRID_SIZE_VAL));
      Value *cy = B.CreateSRem(
          B.CreateAdd(B.CreateAdd(yv, B.getInt32(d[1])),
                      B.getInt32(GRID_SIZE_VAL)),
          B.getInt32(GRID_SIZE_VAL));
      Value *idx = B.CreateAdd(B.CreateMul(cx, B.getInt32(GRID_SIZE_VAL)), cy);
      Value *ptr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), arr, idx);
      Value *val = B.CreateLoad(i32, ptr);
      acc = B.CreateAdd(acc, B.CreateZExt(B.CreateICmpEQ(val, B.getInt32(1)), i32));
    }
    storeReg(B, regTy, regFile, acc, dst);
  });

  // GAME_OF_LIFE_RULE
  fns.ruleFn =
      createVoidFn(m, "ir_GAME_OF_LIFE_RULE", {i32, i32, i32});
  define(fns.ruleFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *dst = it++;
    Value *cur = loadReg(B, regTy, regFile, it++);
    Value *neigh = loadReg(B, regTy, regFile, it++);
    Value *curAlive = B.CreateICmpEQ(cur, B.getInt32(1));
    Value *is2 = B.CreateICmpEQ(neigh, B.getInt32(2));
    Value *is3 = B.CreateICmpEQ(neigh, B.getInt32(3));
    Value *survive = B.CreateOr(is2, is3);
    Value *resAlive =
        B.CreateSelect(curAlive, survive, is3);
    Value *ext = B.CreateZExt(resAlive, B.getInt32Ty());
    storeReg(B, regTy, regFile, ext, dst);
  });

  // RANDOMIZE_CELL
  fns.randFn = createVoidFn(m, "ir_RANDOMIZE_CELL", {i32, i32, i32, i32});
  define(fns.randFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *baseIdx = it++;
    Value *rx = it++;
    Value *ry = it++;
    Value *thr = it++;
    Value *arr = selectArrayPtr(B, curArr, nxtArr, loadReg(B, regTy, regFile, baseIdx));
    Value *xv = loadReg(B, regTy, regFile, rx);
    Value *yv = loadReg(B, regTy, regFile, ry);
    Value *idx = B.CreateAdd(B.CreateMul(xv, B.getInt32(GRID_SIZE_VAL)), yv);
    FunctionCallee randFn = m->getOrInsertFunction(
        "simRand", FunctionType::get(i32, {}, false));
    Value *r = B.CreateSRem(B.CreateCall(randFn), thr);
    Value *alive = B.CreateICmpEQ(r, B.getInt32(0));
    Value *val = B.CreateZExt(alive, i32);
    Value *ptr = B.CreateInBoundsGEP(Type::getInt32Ty(ctx), arr, idx);
    B.CreateStore(val, ptr);
  });

  // DRAW_CELL_4x4
  fns.drawFn = createVoidFn(m, "ir_DRAW_CELL_4x4", {i32, i32, i32});
  define(fns.drawFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *rx = loadReg(B, regTy, regFile, it++);
    Value *ry = loadReg(B, regTy, regFile, it++);
    Value *col = loadReg(B, regTy, regFile, it++);
    FunctionCallee put = m->getOrInsertFunction(
        "simPutPixel", FunctionType::get(Type::getVoidTy(ctx),
                                         {i32, i32, i32}, false));
    Function *f = fn;
    BasicBlock *dyLoop = BasicBlock::Create(ctx, "dy.loop", f);
    BasicBlock *dxLoop = BasicBlock::Create(ctx, "dx.loop", f);
    BasicBlock *after = BasicBlock::Create(ctx, "draw.after", f);
    BasicBlock *pre = B.GetInsertBlock();
    B.CreateBr(dyLoop);

    B.SetInsertPoint(dyLoop);
    PHINode *dy = B.CreatePHI(i32, 2, "dy");
    dy->addIncoming(B.getInt32(0), pre);
    Value *dyCond = B.CreateICmpSLT(dy, B.getInt32(4));
    BasicBlock *dyBody = BasicBlock::Create(ctx, "dy.body", f);
    B.CreateCondBr(dyCond, dyBody, after);

    B.SetInsertPoint(dyBody);
    B.CreateBr(dxLoop);

    B.SetInsertPoint(dxLoop);
    PHINode *dx = B.CreatePHI(i32, 2, "dx");
    dx->addIncoming(B.getInt32(0), dyBody);
    Value *dxCond = B.CreateICmpSLT(dx, B.getInt32(4));
    BasicBlock *dxBody = BasicBlock::Create(ctx, "dx.body", f);
    BasicBlock *dxAfter = BasicBlock::Create(ctx, "dx.after", f);
    B.CreateCondBr(dxCond, dxBody, dxAfter);

    B.SetInsertPoint(dxBody);
    Value *px = B.CreateAdd(B.CreateMul(rx, B.getInt32(4)), dx);
    Value *py = B.CreateAdd(B.CreateMul(ry, B.getInt32(4)), dy);
    B.CreateCall(put, {px, py, col});
    Value *dxNext = B.CreateAdd(dx, B.getInt32(1));
    dx->addIncoming(dxNext, dxBody);
    B.CreateBr(dxLoop);

    B.SetInsertPoint(dxAfter);
    Value *dyNext = B.CreateAdd(dy, B.getInt32(1));
    dy->addIncoming(dyNext, dxAfter);
    B.CreateBr(dyLoop);

    B.SetInsertPoint(after);
  });

  // SELECT_COLOR
  fns.selFn = createVoidFn(m, "ir_SELECT_COLOR", {i32, i32, i32, i32});
  define(fns.selFn, [&](Function *fn, IRBuilder<> &B) {
    auto it = fn->args().begin();
    Value *dst = it++;
    Value *condReg = it++;
    Value *tVal = it++;
    Value *fVal = it++;
    Value *cond = loadReg(B, regTy, regFile, condReg);
    Value *isTrue = B.CreateICmpNE(cond, B.getInt32(0));
    Value *sel = B.CreateSelect(isTrue, tVal, fVal);
    storeReg(B, regTy, regFile, sel, dst);
  });
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    outs() << "[ERROR] Need 1 argument: asm file\n";
    return 1;
  }

  std::ifstream input(argv[1]);
  if (!input.is_open()) {
    outs() << "[ERROR] Can't open " << argv[1] << '\n';
    return 1;
  }

  LLVMContext ctx;
  Module *module = new Module("asm2ir_full", ctx);
  IRBuilder<> builder(ctx);

  ArrayType *regTy = ArrayType::get(builder.getInt32Ty(), REG_FILE_SIZE);
  GlobalVariable *regFile = new GlobalVariable(
      *module, regTy, false, GlobalValue::InternalLinkage,
      ConstantAggregateZero::get(regTy), "regFile");

  ArrayType *arrTy = ArrayType::get(builder.getInt32Ty(), CELL_COUNT);
  GlobalVariable *curArr = new GlobalVariable(
      *module, arrTy, false, GlobalValue::InternalLinkage,
      ConstantAggregateZero::get(arrTy), "curArr");
  GlobalVariable *nxtArr = new GlobalVariable(
      *module, arrTy, false, GlobalValue::InternalLinkage,
      ConstantAggregateZero::get(arrTy), "nxtArr");

  FunctionType *mainTy = FunctionType::get(builder.getVoidTy(), false);
  Function *mainFunc =
      Function::Create(mainTy, Function::ExternalLinkage, "main", module);

  InstrFns fns;
  buildInstrFns(module, fns, regTy, regFile, curArr, nxtArr);

  std::unordered_map<std::string, BasicBlock *> blocks;
  std::string tok;

  auto skipLineIfComment = [&](const std::string &t) {
    if (!t.empty() && t[0] == ';') {
      std::string dummy;
      std::getline(input, dummy);
      return true;
    }
    return false;
  };

  // Pass 1: labels
  while (input >> tok) {
    if (skipLineIfComment(tok))
      continue;
    if (is_label(tok)) {
      std::string label = tok.substr(0, tok.size() - 1);
      blocks[label] = BasicBlock::Create(ctx, label, mainFunc);
    } else {
      std::getline(input, tok);
    }
  }

  input.clear();
  input.seekg(0);
  BasicBlock *entryBB = nullptr;
  auto itEntry = blocks.find("entry");
  if (itEntry != blocks.end()) {
    entryBB = itEntry->second;
  } else {
    entryBB = BasicBlock::Create(ctx, "entry", mainFunc);
  }
  builder.SetInsertPoint(entryBB);

  auto toReg = [](const std::string &s) { return std::stoi(s.substr(1)); };
  auto toImm = [](const std::string &s) {
    long long v = std::stoll(s, nullptr, 0);
    return static_cast<int32_t>(v);
  };

  auto regConst = [&](int idx) { return builder.getInt32(idx); };

  auto branchToLabel = [&](const std::string &label) {
    builder.CreateBr(blocks[label]);
    builder.SetInsertPoint(blocks[label]);
  };

  while (input >> tok) {
    if (skipLineIfComment(tok))
      continue;
    if (is_label(tok)) {
      std::string label = tok.substr(0, tok.size() - 1);
      BasicBlock *target = blocks[label];
      BasicBlock *cur = builder.GetInsertBlock();
      if (cur && !cur->getTerminator() && cur != target)
        builder.CreateBr(target);
      builder.SetInsertPoint(target);
      continue;
    }
    if (tok == "EXIT") {
      builder.CreateRetVoid();
      continue;
    }
    if (tok == "BR") {
      std::string target;
      input >> target;
      builder.CreateBr(blocks[target]);
      continue;
    }
    if (tok == "BR_COND") {
      std::string condReg, target;
      input >> condReg >> target;
      Value *cond =
          loadReg(builder, regTy, regFile, regConst(toReg(condReg)));
      Value *isTrue =
          builder.CreateICmpNE(cond, builder.getInt32(0), "br.cond");
      BasicBlock *trueBB = blocks[target];
      BasicBlock *falseBB =
          BasicBlock::Create(ctx, "fallthrough", mainFunc);
      builder.CreateCondBr(isTrue, trueBB, falseBB);
      builder.SetInsertPoint(falseBB);
      continue;
    }
    // Regular instructions -> call internal IR functions
    auto call3 = [&](FunctionCallee fn, const std::string &a1,
                     const std::string &a2, const std::string &a3,
                     bool imm3 = false) {
      Value *args[] = {regConst(toReg(a1)), regConst(toReg(a2)),
                       imm3 ? builder.getInt32(toImm(a3))
                            : regConst(toReg(a3))};
      builder.CreateCall(fn, args);
    };
    auto call4 = [&](FunctionCallee fn, const std::string &a1,
                     const std::string &a2, const std::string &a3,
                     const std::string &a4, bool imm3 = false, bool imm4 = false) {
      Value *args[] = {
          regConst(toReg(a1)),
          regConst(toReg(a2)),
          imm3 ? builder.getInt32(toImm(a3)) : regConst(toReg(a3)),
          imm4 ? builder.getInt32(toImm(a4)) : regConst(toReg(a4))};
      builder.CreateCall(fn, args);
    };

    if (tok == "XOR") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      call3(fns.xorFn, a1, a2, a3);
      continue;
    }
    if (tok == "MUL") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      call3(fns.mulFn, a1, a2, a3);
      continue;
    }
    if (tok == "SUBi") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      call3(fns.subiFn, a1, a2, a3, true);
      continue;
    }
    if (tok == "INC_NEi") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      call3(fns.incFn, a1, a2, a3, true);
      continue;
    }
    if (tok == "PUT_PIXEL") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      call3(fns.putFn, a1, a2, a3);
      continue;
    }
    if (tok == "FLUSH") {
      builder.CreateCall(fns.flushFn);
      continue;
    }
    if (tok == "ALLOC_ARRAYS") {
      std::string a1, a2;
      input >> a1 >> a2;
      Value *args[] = {regConst(toReg(a1)), regConst(toReg(a2))};
      builder.CreateCall(fns.allocFn, args);
      continue;
    }
    if (tok == "CLEAR_ARRAY") {
      std::string base, len;
      input >> base >> len;
      Value *args[] = {regConst(toReg(base)), builder.getInt32(toImm(len))};
      builder.CreateCall(fns.clearFn, args);
      continue;
    }
    if (tok == "COPY_ARRAY") {
      std::string dst, src, len;
      input >> dst >> src >> len;
      Value *args[] = {regConst(toReg(dst)), regConst(toReg(src)),
                       builder.getInt32(toImm(len))};
      builder.CreateCall(fns.copyFn, args);
      continue;
    }
    if (tok == "GET_CELL") {
      std::string dst, base, x, y;
      input >> dst >> base >> x >> y;
      call4(fns.getFn, dst, base, x, y);
      continue;
    }
    if (tok == "SET_CELL") {
      std::string base, x, y, val;
      input >> base >> x >> y >> val;
      call4(fns.setFn, base, x, y, val);
      continue;
    }
    if (tok == "COUNT_NEIGHBORS") {
      std::string dst, base, x, y;
      input >> dst >> base >> x >> y;
      call4(fns.cntFn, dst, base, x, y);
      continue;
    }
    if (tok == "GAME_OF_LIFE_RULE") {
      std::string dst, cur, neigh;
      input >> dst >> cur >> neigh;
      call3(fns.ruleFn, dst, cur, neigh);
      continue;
    }
    if (tok == "RANDOMIZE_CELL") {
      std::string base, x, y, thr;
      input >> base >> x >> y >> thr;
      call4(fns.randFn, base, x, y, thr, false, true);
      continue;
    }
    if (tok == "DRAW_CELL_4x4") {
      std::string x, y, c;
      input >> x >> y >> c;
      call3(fns.drawFn, x, y, c);
      continue;
    }
    if (tok == "SELECT_COLOR") {
      std::string dst, cond, ct, cf;
      input >> dst >> cond >> ct >> cf;
      call4(fns.selFn, dst, cond, ct, cf, true, true);
      continue;
    }
    outs() << "[WARN] Unknown token " << tok << "\n";
  }

  bool verif = verifyFunction(*mainFunc, &outs());
  outs() << "[VERIFICATION] " << (verif ? "FAIL" : "OK") << "\n";
  outs() << "\n#[LLVM IR]:\n";
  module->print(outs(), nullptr);

  outs() << "[RUN] Execution disabled in asm2ir_full (IR generation only).\n";
  return 0;
}
