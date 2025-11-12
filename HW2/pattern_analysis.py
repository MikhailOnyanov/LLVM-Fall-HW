#!/usr/bin/env python3
import collections
import re
import sys
from tqdm import tqdm


def analyze_patterns(trace_file, max_pattern_length=5):
    instructions = []

    call_re = re.compile(r"CALL '([^']+)' -> '([^']+)'")
    binop_re = re.compile(r"In function '([^']+)': ([^=]+)=([^a-zA-Z0-9]*)\s*(\S+)\s+(\S+)")

    # --- Считаем количество строк для прогресс-бара чтения ---
    with open(trace_file, 'r') as f:
        total_lines = sum(1 for _ in f)

    with open(trace_file, 'r') as f:
        for line in tqdm(f, total=total_lines, desc="Чтение и парсинг трассы"):
            line = line.strip()

            # CALL pattern: CALL 'A' -> 'B'
            if "CALL" in line:
                m = call_re.search(line)
                if m:
                    caller, callee = m.groups()
                    instructions.append((caller, callee))
                    continue

            # Binary operation pattern: X = Y op Z
            if "In function" in line and "=" in line:
                try:
                    parts = line.split(":", 1)[1].strip()
                    lhs, rhs = parts.split("=", 1)
                    lhs = lhs.strip()
                    rhs = rhs.strip()
                    instructions.append((lhs, rhs))
                except ValueError:
                    continue

    pattern_stats = collections.Counter()

    # --- Прогресс бар для подсчета паттернов ---
    for length in tqdm(range(1, max_pattern_length + 1), desc="Анализ паттернов"):
        for i in tqdm(range(len(instructions) - length + 1), leave=False, desc=f"Длина {length}"):
            pattern = tuple(instructions[i:i + length])
            pattern_stats[pattern] += 1

    return pattern_stats.most_common()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python analyze_patterns.py <trace_file>")
        sys.exit(1)

    patterns = analyze_patterns(sys.argv[1])

    print("\nTop patterns found:")
    for pattern, count in patterns[:20]:
        print(f"Count: {count}")
        for user, operand in pattern:
            print(f"  {user} <- {operand}")
        print()
