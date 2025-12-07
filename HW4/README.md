# Четвертое задание: 
1) Свой набор инструкций из статистики для ASM и LLVM IR вашего приложения (описание инструкций выложить в репозиторий) 
2) Переписать приложение на ASM (скинуть в репозиторий)
 
# Написать ASM2IR генератор: 
3) С вызовами эмулирующих функций (кроме инструкций потока управления) 
4) С генерацией IR эквивалентов для всех инструкций

Примеры:
1) https://github.com/lisitsynSA/llvm_course/blob/main/SDL/IRGen/app.s
2) https://github.com/lisitsynSA/llvm_course/blob/main/SDL/IRGen/app_asm_IRgen_1.cpp
3) https://github.com/lisitsynSA/llvm_course/blob/main/SDL/IRGen/app_asm_IRgen_2.cpp

Продвинутый пример (1: симуляция, 2: IR c внешними функциями, 3: только IR):
https://github.com/lisitsynSA/llvm_course/tree/main/Sim

# Instruction set observed / used

Для каждой инструкции — краткое описание и IR-эквивалент.

## 1) Служебные LLVM-инструкции (из трассы)
- `llvm.lifetime.start.p0` / `llvm.lifetime.end.p0`
  - Описание: маркеры жизненного цикла временных объектов; вставляются LLVM.
  - Пример IR: `call void @llvm.lifetime.start.p0(i64 8, i8* %ptr)`
  - Комментарий: шум для паттерн-анализа; можно фильтровать.

## 2) Вызовы
- `CALL 'A' -> 'B'` (в трассе)
  - Описание: вызов функции `B` из `A`.
  - IR: `call <rettype> @B(<args>)`

## 3) Бинарные операции (LLVM IR)
- `add`, `sub`, `mul`, `sdiv`, `srem`
  - Описание: целочисленные операции.
  - IR-эквивалент: `res = add i64 %a, %b` и т.п.

## 4) ASM (aarch64) мнемоники и эквиваленты
Ниже — наиболее часто встречающиеся мнемоники и то, как они переводятся в IR / эмулируются.

- `add xD, xN, xM`  → IR: `%d = add i64 %n, %m` ; emu: `call i64 @emu_add(i64 %n, i64 %m)`
- `sub xD, xN, xM`  → IR: `%d = sub i64 %n, %m` ; emu: `call i64 @emu_sub(i64 %n, i64 %m)`
- `mul xD, xN, xM`  → IR: `%d = mul i64 %n, %m` ; emu: `call i64 @emu_mul(i64 %n, i64 %m)`
- `sdiv xD, xN, xM` → IR: `%d = sdiv i64 %n, %m` ; emu: `call i64 @emu_sdiv(i64 %n, i64 %m)`
- `srem xD, xN, xM` → IR: `%d = srem i64 %n, %m` ; emu: `call i64 @emu_srem(i64 %n, i64 %m)`

- `mov` / `orr` used as move → IR: `bitcast/store` or `%d = or i64 %n, 0` ; emu: `call i64 @emu_mov(i64 %n)`

- `ldr xD, [xN, #imm]` → emu: `call i64 @emu_load(i64 %xN, i64 imm)` ; IR: `%d = call i64 @emu_load(i64 %xN, i64 imm)`
- `str xD, [xN, #imm]` → emu: `call void @emu_store(i64 %xN, i64 imm, i64 %xD)` ; IR: `call void @emu_store(...)`

- `bl <label|func>` → IR: `call <ret> @<func>` ; emulator: also emit `call void @emu_call("func")` if needed.

- `cmp` / `subs` followed by conditional branches: *инструкции потока управления* — по заданию их **не эмулируем**, но в генераторе их можно оставить как метки/комментарии.

## 5) Комментарии по нормализации и фильтрации
- `llvm.lifetime.*` — шум. Для статистики паттернов рекомендую исключать или поместить в отдельную категорию.
- Идентификаторы инструкций (`{5257...}`) заменены на нормализованные ID при подготовке данных отчёта (иначе паттерны дробятся).
