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
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace llvm;

static const int REG_FILE_SIZE = 16;
static const int GRID_SIZE_VAL = 128;
static const int CELL_COUNT = GRID_SIZE_VAL * GRID_SIZE_VAL;

static int32_t REG_FILE[REG_FILE_SIZE];
static std::vector<int32_t> CUR(CELL_COUNT);
static std::vector<int32_t> NXT(CELL_COUNT);

// Helpers for emulation mode (one do_* per instruction)
static std::vector<int32_t> &select_array(int32_t base) {
  return base == 0 ? CUR : NXT;
}

extern "C" void do_XOR(int arg1, int arg2, int arg3) {
  REG_FILE[arg1] = REG_FILE[arg2] ^ REG_FILE[arg3];
}

extern "C" void do_MUL(int arg1, int arg2, int arg3) {
  REG_FILE[arg1] = REG_FILE[arg2] * REG_FILE[arg3];
}

extern "C" void do_SUBi(int arg1, int arg2, int arg3) {
  REG_FILE[arg1] = REG_FILE[arg2] - arg3;
}

extern "C" void do_INC_NEi(int arg1, int arg2, int arg3) {
  REG_FILE[arg2]++;
  REG_FILE[arg1] = REG_FILE[arg2] != arg3;
}

extern "C" void do_PUT_PIXEL(int arg1, int arg2, int arg3) {
  simPutPixel(REG_FILE[arg1], REG_FILE[arg2], REG_FILE[arg3]);
}

extern "C" void do_FLUSH() { simFlush(); }

extern "C" void do_ALLOC_ARRAYS(int arg1, int arg2) {
  REG_FILE[arg1] = 0; // CUR id
  REG_FILE[arg2] = 1; // NXT id
}

extern "C" void do_CLEAR_ARRAY(int arg1, int arg2) {
  auto &arr = select_array(REG_FILE[arg1]);
  const int len = arg2;
  for (int i = 0; i < len; ++i)
    arr[i] = 0;
}

extern "C" void do_COPY_ARRAY(int arg1, int arg2, int arg3) {
  auto &dst = select_array(REG_FILE[arg1]);
  auto &src = select_array(REG_FILE[arg2]);
  const int len = arg3;
  for (int i = 0; i < len; ++i)
    dst[i] = src[i];
}

extern "C" void do_GET_CELL(int arg1, int arg2, int arg3, int arg4) {
  auto &arr = select_array(REG_FILE[arg2]);
  int idx = REG_FILE[arg3] * GRID_SIZE_VAL + REG_FILE[arg4];
  REG_FILE[arg1] = arr[idx];
}

extern "C" void do_SET_CELL(int arg1, int arg2, int arg3, int arg4) {
  auto &arr = select_array(REG_FILE[arg1]);
  int idx = REG_FILE[arg2] * GRID_SIZE_VAL + REG_FILE[arg3];
  arr[idx] = REG_FILE[arg4];
}

extern "C" void do_COUNT_NEIGHBORS(int arg1, int arg2, int arg3, int arg4) {
  auto &arr = select_array(REG_FILE[arg2]);
  int x = REG_FILE[arg3];
  int y = REG_FILE[arg4];
  int count = 0;
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0)
        continue;
      int cx = (x + dx + GRID_SIZE_VAL) % GRID_SIZE_VAL;
      int cy = (y + dy + GRID_SIZE_VAL) % GRID_SIZE_VAL;
      int idx = cx * GRID_SIZE_VAL + cy;
      if (arr[idx] == 1)
        count++;
    }
  }
  REG_FILE[arg1] = count;
}

extern "C" void do_GAME_OF_LIFE_RULE(int arg1, int arg2, int arg3) {
  int cur = REG_FILE[arg2];
  int neigh = REG_FILE[arg3];
  int res = 0;
  if (cur == 1) {
    res = (neigh == 2 || neigh == 3);
  } else {
    res = (neigh == 3);
  }
  REG_FILE[arg1] = res;
}

extern "C" void do_RANDOMIZE_CELL(int arg1, int arg2, int arg3, int arg4) {
  auto &arr = select_array(REG_FILE[arg1]);
  int x = REG_FILE[arg2];
  int y = REG_FILE[arg3];
  int idx = x * GRID_SIZE_VAL + y;
  int alive = (simRand() % arg4 == 0) ? 1 : 0;
  arr[idx] = alive;
}

extern "C" void do_DRAW_CELL_4x4(int arg1, int arg2, int arg3) {
  int x = REG_FILE[arg1];
  int y = REG_FILE[arg2];
  int color = REG_FILE[arg3];
  for (int dy = 0; dy < 4; ++dy)
    for (int dx = 0; dx < 4; ++dx)
      simPutPixel(x * 4 + dx, y * 4 + dy, color);
}

extern "C" void do_SELECT_COLOR(int arg1, int arg2, int arg3, int arg4) {
  REG_FILE[arg1] = REG_FILE[arg2] ? arg3 : arg4;
}

static bool is_label(const std::string &name) {
  return !name.empty() && name.back() == ':';
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

  LLVMContext context;
  Module *module = new Module("asm2ir_emul", context);
  IRBuilder<> builder(context);

  ArrayType *regTy = ArrayType::get(builder.getInt32Ty(), REG_FILE_SIZE);
  module->getOrInsertGlobal("regFile", regTy);
  GlobalVariable *regFile = module->getNamedGlobal("regFile");

  FunctionType *mainTy = FunctionType::get(builder.getVoidTy(), false);
  Function *mainFunc =
      Function::Create(mainTy, Function::ExternalLinkage, "main", module);

  std::string name;
  std::vector<std::string> tokens;
  std::unordered_map<std::string, BasicBlock *> blocks;

  // Pass 1: collect labels
  auto skipLineIfComment = [&](const std::string &tok) {
    if (!tok.empty() && tok[0] == ';') {
      std::string dummy;
      std::getline(input, dummy);
      return true;
    }
    return false;
  };

  while (input >> name) {
    if (skipLineIfComment(name))
      continue;
    if (is_label(name)) {
      std::string label = name.substr(0, name.size() - 1);
      blocks[label] = BasicBlock::Create(context, label, mainFunc);
    } else {
      std::getline(input, name); // skip rest of line
    }
  }

  input.clear();
  input.seekg(0);
  BasicBlock *entryBB = nullptr;
  auto itEntry = blocks.find("entry");
  if (itEntry != blocks.end()) {
    entryBB = itEntry->second;
  } else {
    entryBB = BasicBlock::Create(context, "entry", mainFunc);
  }
  builder.SetInsertPoint(entryBB);

  auto regPtr = [&](int idx) {
    return builder.CreateConstGEP2_32(regTy, regFile, 0, idx);
  };
  auto regI32 = [&](int idx) {
    Value *ptr = regPtr(idx);
    return builder.CreateLoad(builder.getInt32Ty(), ptr);
  };
  auto regStore = [&](Value *val, int idx) {
    Value *ptr = regPtr(idx);
    builder.CreateStore(val, ptr);
  };

  // Function declarations for do_* emulation
  Type *i32 = builder.getInt32Ty();
  Type *voidTy = builder.getVoidTy();
  FunctionType *fn_i32x3 = FunctionType::get(voidTy, {i32, i32, i32}, false);
  FunctionType *fn_i32x4 =
      FunctionType::get(voidTy, {i32, i32, i32, i32}, false);
  auto decl = [&](StringRef n, FunctionType *ty) {
    return module->getOrInsertFunction(n, ty);
  };
  auto fnXOR = decl("do_XOR", fn_i32x3);
  auto fnMUL = decl("do_MUL", fn_i32x3);
  auto fnSUBi = decl("do_SUBi", fn_i32x3);
  auto fnINC = decl("do_INC_NEi", fn_i32x3);
  auto fnPP = decl("do_PUT_PIXEL", fn_i32x3);
  auto fnFlush = decl("do_FLUSH", FunctionType::get(voidTy, {}, false));
  auto fnAlloc =
      decl("do_ALLOC_ARRAYS", FunctionType::get(voidTy, {i32, i32}, false));
  auto fnClear = decl("do_CLEAR_ARRAY", FunctionType::get(voidTy, {i32, i32},
                                                          false));
  auto fnCopy =
      decl("do_COPY_ARRAY", FunctionType::get(voidTy, {i32, i32, i32}, false));
  auto fnGet = decl("do_GET_CELL", fn_i32x4);
  auto fnSet = decl("do_SET_CELL", fn_i32x4);
  auto fnCnt = decl("do_COUNT_NEIGHBORS", fn_i32x4);
  auto fnRule = decl("do_GAME_OF_LIFE_RULE", FunctionType::get(
                                                 voidTy, {i32, i32, i32}, false));
  auto fnRand = decl("do_RANDOMIZE_CELL", fn_i32x4);
  auto fnDraw = decl("do_DRAW_CELL_4x4", fn_i32x3);
  auto fnSel = decl("do_SELECT_COLOR", fn_i32x4);

  // Pass 2: emit IR
  auto toImm = [](const std::string &tok) {
    long long v = std::stoll(tok, nullptr, 0);
    return static_cast<int32_t>(v);
  };
  auto toReg = [](const std::string &tok) { return std::stoi(tok.substr(1)); };

  auto setNextBlock = [&](const std::string &label) {
    auto it = blocks.find(label);
    assert(it != blocks.end() && "unknown label");
    builder.SetInsertPoint(it->second);
  };

  input.clear();
  input.seekg(0);
  while (input >> name) {
    if (skipLineIfComment(name))
      continue;
    if (is_label(name)) {
      std::string label = name.substr(0, name.size() - 1);
      BasicBlock *target = blocks[label];
      BasicBlock *cur = builder.GetInsertBlock();
      if (cur && !cur->getTerminator() && cur != target) {
        builder.CreateBr(target);
      }
      builder.SetInsertPoint(target);
      continue;
    }

    if (name == "EXIT") {
      builder.CreateRetVoid();
      continue;
    }
    if (name == "BR") {
      input >> name; // target
      builder.CreateBr(blocks[name]);
      continue;
    }
    if (name == "BR_COND") {
      std::string condReg, target;
      input >> condReg >> target;
      Value *condPtr = regPtr(toReg(condReg));
      Value *i32Cond = builder.CreateLoad(i32, condPtr);
      Value *i1Cond = builder.CreateICmpNE(i32Cond, builder.getInt32(0));
      BasicBlock *trueBB = blocks[target];
      // false -> fallthrough: create stub block if none set
      BasicBlock *falseBB = BasicBlock::Create(context, "fallthrough", mainFunc);
      builder.CreateCondBr(i1Cond, trueBB, falseBB);
      builder.SetInsertPoint(falseBB);
      continue;
    }
    if (name == "XOR" || name == "MUL" || name == "SUBi" ||
        name == "INC_NEi") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      Value *args[] = {builder.getInt32(toReg(a1)), builder.getInt32(toReg(a2)),
                       builder.getInt32(name == "SUBi" || name == "INC_NEi"
                                            ? toImm(a3)
                                            : toReg(a3))};
      if (name == "XOR")
        builder.CreateCall(fnXOR, args);
      else if (name == "MUL")
        builder.CreateCall(fnMUL, args);
      else if (name == "SUBi")
        builder.CreateCall(fnSUBi, args);
      else
        builder.CreateCall(fnINC, args);
      continue;
    }
    if (name == "PUT_PIXEL") {
      std::string a1, a2, a3;
      input >> a1 >> a2 >> a3;
      Value *args[] = {builder.getInt32(toReg(a1)), builder.getInt32(toReg(a2)),
                       builder.getInt32(toReg(a3))};
      builder.CreateCall(fnPP, args);
      continue;
    }
    if (name == "FLUSH") {
      builder.CreateCall(fnFlush);
      continue;
    }
    if (name == "ALLOC_ARRAYS") {
      std::string a1, a2;
      input >> a1 >> a2;
      Value *args[] = {builder.getInt32(toReg(a1)), builder.getInt32(toReg(a2))};
      builder.CreateCall(fnAlloc, args);
      continue;
    }
    if (name == "CLEAR_ARRAY") {
      std::string base, len;
      input >> base >> len;
      Value *args[] = {builder.getInt32(toReg(base)),
                       builder.getInt32(toImm(len))};
      builder.CreateCall(fnClear, args);
      continue;
    }
    if (name == "COPY_ARRAY") {
      std::string dst, src, len;
      input >> dst >> src >> len;
      Value *args[] = {builder.getInt32(toReg(dst)), builder.getInt32(toReg(src)),
                       builder.getInt32(toImm(len))};
      builder.CreateCall(fnCopy, args);
      continue;
    }
    if (name == "GET_CELL") {
      std::string dst, base, x, y;
      input >> dst >> base >> x >> y;
      Value *args[] = {builder.getInt32(toReg(dst)), builder.getInt32(toReg(base)),
                       builder.getInt32(toReg(x)), builder.getInt32(toReg(y))};
      builder.CreateCall(fnGet, args);
      continue;
    }
    if (name == "SET_CELL") {
      std::string base, x, y, val;
      input >> base >> x >> y >> val;
      Value *args[] = {builder.getInt32(toReg(base)), builder.getInt32(toReg(x)),
                       builder.getInt32(toReg(y)), builder.getInt32(toReg(val))};
      builder.CreateCall(fnSet, args);
      continue;
    }
    if (name == "COUNT_NEIGHBORS") {
      std::string dst, base, x, y;
      input >> dst >> base >> x >> y;
      Value *args[] = {builder.getInt32(toReg(dst)), builder.getInt32(toReg(base)),
                       builder.getInt32(toReg(x)), builder.getInt32(toReg(y))};
      builder.CreateCall(fnCnt, args);
      continue;
    }
    if (name == "GAME_OF_LIFE_RULE") {
      std::string dst, cur, neigh;
      input >> dst >> cur >> neigh;
      Value *args[] = {builder.getInt32(toReg(dst)), builder.getInt32(toReg(cur)),
                       builder.getInt32(toReg(neigh))};
      builder.CreateCall(fnRule, args);
      continue;
    }
    if (name == "RANDOMIZE_CELL") {
      std::string base, x, y, thr;
      input >> base >> x >> y >> thr;
      Value *args[] = {builder.getInt32(toReg(base)), builder.getInt32(toReg(x)),
                       builder.getInt32(toReg(y)), builder.getInt32(toImm(thr))};
      builder.CreateCall(fnRand, args);
      continue;
    }
    if (name == "DRAW_CELL_4x4") {
      std::string x, y, c;
      input >> x >> y >> c;
      Value *args[] = {builder.getInt32(toReg(x)), builder.getInt32(toReg(y)),
                       builder.getInt32(toReg(c))};
      builder.CreateCall(fnDraw, args);
      continue;
    }
    if (name == "SELECT_COLOR") {
      std::string dst, cond, ct, cf;
      input >> dst >> cond >> ct >> cf;
      Value *args[] = {builder.getInt32(toReg(dst)), builder.getInt32(toReg(cond)),
                       builder.getInt32(toImm(ct)), builder.getInt32(toImm(cf))};
      builder.CreateCall(fnSel, args);
      continue;
    }
    outs() << "[WARN] Unknown token: " << name << "\n";
  }

  bool verif = verifyFunction(*mainFunc, &outs());
  outs() << "[VERIFICATION] " << (verif ? "FAIL" : "OK") << "\n";

  outs() << "\n#[LLVM IR]:\n";
  module->print(outs(), nullptr);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  ExecutionEngine *ee = EngineBuilder(std::unique_ptr<Module>(module)).create();
  ee->InstallLazyFunctionCreator([](const std::string &fnName) -> void * {
    auto match = [&](const char *name) {
      return fnName == name || (fnName.size() > 1 && fnName[0] == '_' &&
                                fnName.substr(1) == name);
    };
    if (match("do_XOR"))
      return reinterpret_cast<void *>(do_XOR);
    if (match("do_MUL"))
      return reinterpret_cast<void *>(do_MUL);
    if (match("do_SUBi"))
      return reinterpret_cast<void *>(do_SUBi);
    if (match("do_INC_NEi"))
      return reinterpret_cast<void *>(do_INC_NEi);
    if (match("do_PUT_PIXEL"))
      return reinterpret_cast<void *>(do_PUT_PIXEL);
    if (match("do_FLUSH"))
      return reinterpret_cast<void *>(do_FLUSH);
    if (match("do_ALLOC_ARRAYS"))
      return reinterpret_cast<void *>(do_ALLOC_ARRAYS);
    if (match("do_CLEAR_ARRAY"))
      return reinterpret_cast<void *>(do_CLEAR_ARRAY);
    if (match("do_COPY_ARRAY"))
      return reinterpret_cast<void *>(do_COPY_ARRAY);
    if (match("do_GET_CELL"))
      return reinterpret_cast<void *>(do_GET_CELL);
    if (match("do_SET_CELL"))
      return reinterpret_cast<void *>(do_SET_CELL);
    if (match("do_COUNT_NEIGHBORS"))
      return reinterpret_cast<void *>(do_COUNT_NEIGHBORS);
    if (match("do_GAME_OF_LIFE_RULE"))
      return reinterpret_cast<void *>(do_GAME_OF_LIFE_RULE);
    if (match("do_RANDOMIZE_CELL"))
      return reinterpret_cast<void *>(do_RANDOMIZE_CELL);
    if (match("do_DRAW_CELL_4x4"))
      return reinterpret_cast<void *>(do_DRAW_CELL_4x4);
    if (match("do_SELECT_COLOR"))
      return reinterpret_cast<void *>(do_SELECT_COLOR);
    if (match("simRand"))
      return reinterpret_cast<void *>(simRand);
    if (match("simPutPixel"))
      return reinterpret_cast<void *>(simPutPixel);
    if (match("simFlush"))
      return reinterpret_cast<void *>(simFlush);
    outs() << "[ExecutionEngine] Unknown function " << fnName << "\n";
    return nullptr;
  });
  ee->addGlobalMapping(regFile, (void *)REG_FILE);
  ee->finalizeObject();

  simInit();
  ee->runFunction(mainFunc, {});
  simExit();
  return 0;
}
