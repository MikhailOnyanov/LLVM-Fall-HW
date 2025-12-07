import argparse
from collections import Counter
from pathlib import Path


def collect_patterns(trace_path: Path, max_len: int, top: int, out_path: Path) -> None:
    if not trace_path.exists():
        raise SystemExit(f"Trace file not found: {trace_path}")

    instructions = [
        line.strip()
        for line in trace_path.read_text().splitlines()
        if line.strip()
    ]

    lines: list[str] = []

    for size in range(1, max_len + 1):
        counter: Counter[tuple[str, ...]] = Counter()
        for idx in range(len(instructions) - size + 1):
            window = tuple(instructions[idx : idx + size])
            counter[window] += 1

        lines.append(f"\nTop patterns of length {size}:")
        for pattern, count in counter.most_common(top):
            printable = " -> ".join(pattern)
            lines.append(f"{count:5} | {printable}")

    report = "\n".join(lines) + "\n"
    print(report, end="")
    out_path.write_text(report)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Count frequent IR instruction patterns (length 1-5)."
    )
    parser.add_argument(
        "--trace",
        type=Path,
        default=Path("ir_trace.log"),
        help="Path to the instruction trace file (default: ir_trace.log)",
    )
    parser.add_argument(
        "--max-len",
        type=int,
        default=5,
        help="Maximum pattern length to evaluate (default: 5).",
    )
    parser.add_argument(
        "--top",
        type=int,
        default=20,
        help="How many top patterns to print per length (default: 20).",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=Path("patterns_report.txt"),
        help="Where to save the analysis report (default: patterns_report.txt).",
    )
    args = parser.parse_args()
    collect_patterns(args.trace, args.max_len, args.top, args.out)


if __name__ == "__main__":
    main()
