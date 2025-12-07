# Второе задание:

С помощью инструментирующего Pass собрать (в рантайме) трассу исполненных IR инструкций / трассу использования инструкций (User <- Operand) графического приложения (только для логического модуля - app.c) на -O1/2/3/s (пропуская User, если это phi*). Код Pass выложить в репозиторий. 

Провести анализ часто повторяемых паттернов (длина паттерна: 1-5 инструкций). Собранную статистику выложить в репозиторий.

## Задание со звёздочкой: 
При нахождении операнда из инструкции phi, печатать инструкции, используемые в операндах phi.
Пример: запись shl <- phi заменяется на две записи shl <- add и shl <- sub, если этот phi  использует в качестве операндов add и  sub.

## Как собрать и запустить
```sh
# Сборка плагина
clang++ -fPIC -shared AppIRTracePass.cpp -o libAppIRTrace.so \
    `llvm-config --cxxflags --ldflags --system-libs --libs core passes irreader`

# Инструментирование только app.c (пример с -O2; аналогично -O1/-O3/-Os)
clang ../HW1/app.c -c -fpass-plugin=./libAppIRTrace.so -O2

# Компиляция и линковка остальных файлов
clang ../HW1/start.c ../HW1/sim.c ../HW1/grid.c ./log.c ./allInstrLogger.c app.o \
    -I/opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2 \
    -L/opt/homebrew/Cellar/sdl2/2.32.10/lib -lSDL2 -O2
```

При выполнении бинарника появятся ir_trace.log и ir_use.log. Для анализа паттернов:
```sh
python3 pattern_analysis.py --trace ir_trace.log --max-len 5 --top 20
```
