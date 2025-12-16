# HW4 — ASM и ASM2IR

- `ISA.md`: макро-ISA на регистрах x0–x15.
- `app_game_of_life.s`: моя game-of-life на этом ASM (переделанный app.c).
- `asm2ir_emul.cpp`: генератор, который превращает ASM в IR с вызовами `do_*`. 
- `asm2ir_full.cpp`: генератор полного IR без эмуляции.
- `Makefile`: сборка под macOS; пути к SDL2 можно переопределить через `SDL_INC`/`SDL_LIB`. LLVM берётся из `llvm-config`.

## Как собрать
```sh
cd HW4
make            # asm2ir_emul и asm2ir_full
```

## Как запускать
- Эмуляция (выполняет логику игры):
  ```sh
  make run_emul ASM=app_game_of_life.s
  ```
- Полный IR (печатает IR, без исполнения):
  ```sh
  make run_full ASM=app_game_of_life.s
  ```
