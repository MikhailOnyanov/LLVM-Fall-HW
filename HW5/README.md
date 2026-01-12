# HW5 - PixelFlow (Flex/Bison + LLVM IR)

PixelFlow - небольшой язык для графических приложений с компиляцией в LLVM IR.

## Особенности
- Тип данных: `int32`
- Массивы фиксированного размера: `array name[size];`
- Функции с 0+ параметрами, возврат через именованную переменную
- Управляющие конструкции: `for`, `if/else`
- Встроенные функции: `pixel`, `show`, `rand`, `rgb`, `assert`

## Грамматика ЯП
```
program     -> functionDecl+
functionDecl-> 'fun' ID '(' params? ')' returnVar? block
params      -> ID (',' ID)*
returnVar   -> ID
block       -> '{' statement* '}'
statement   -> assignment
            | arrayDecl
            | arrayAssign
            | forLoop
            | ifStmt
            | call ';'
assignment  -> ID '=' expr ';'
arrayDecl   -> 'array' ID '[' expr ']' ';'
arrayAssign -> ID '[' expr ']' '=' expr ';'
forLoop     -> 'for' ID 'in' expr '..' expr block
ifStmt      -> 'if' '(' expr ')' block ('else' block)?
call        -> builtins | ID '(' args? ')'
expr        -> term (('+'|'-'|'<'|'>'|'<='|'>='|'=='|'!=') term)*
term        -> factor (('*'|'/'|'%') factor)*
factor      -> NUMBER | ID | ID '[' expr ']' | '(' expr ')' | '-' factor | 'rand()' | call
```

## Точка входа
Программа должна содержать функцию `app()`.

## Пример (Game of Life)
Файл `game_of_life.pflow` содержит реализацию клеточного автомата.

## Сборка и запуск
```
make run
```
- Генерация LLVM IR
- Сборка в бинарник через `clang`
- Запуск приложения

### JIT (ExecutionEngine)
```
make exec
```

### Тесты
```
make test
```

## Файлы
- `PixelFlow.lex` - лексер Flex
- `PixelFlow.y` - грамматика Bison + генерация LLVM IR
- `game_of_life.pflow` - демо-приложение
- `tests/` - тесты языка
