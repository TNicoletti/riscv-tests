#!/usr/bin/env python3
"""RVV instruction coverage from a Spike commit log.

Usage:
  spike -l --log-commits --log=run.log --isa=rv64gcv_zvl128b_zicntr_zba_zbb prog.riscv
  python3 test_runs/rvv_coverage.py run.log [--spike-src ~/Documents/Mestrado/riscv-isa-sim] [--csv out.csv]

Reports:
  - which RVV instructions (from Spike's own riscv_insn_ext_v list) were executed
  - which (instruction, SEW, LMUL) combinations were exercised
  - which pairs and triples of consecutive RVV instructions were exercised, out of
    all N^2 / N^3 combinations of the N covered instructions (scalar instructions in
    between are ignored; use --exclude to drop e.g. vsetvli/vle32_v/vse32_v)
"""
import argparse
import collections
import csv
import os
import re
import sys

# Disassembly line:  core   0: 0x...  (0x01857557) vsetvli a0, a0, e64, m1, tu, mu
DISASM_RE = re.compile(r"^core\s+\d+:\s+0x[0-9a-f]+\s+\(0x[0-9a-f]+\)\s+(\S+)")
# Commit line:       core   0: 3 0x...  (0x0202f007) e64 m1 l2 v0 ...
COMMIT_RE = re.compile(r"^core\s+\d+:\s+\d+\s+0x[0-9a-f]+\s+\(0x[0-9a-f]+\)\s+e(\d+)\s+(mf?\d+)\s+l(\d+)")

# Segment variants share the encoding (nf field) of their base instruction in Spike
SEGMENT_RE = re.compile(r"^v(l|s)(s|ux|ox)?seg\d(e\d+(ff)?|ei\d+)_v$")


def load_reference(spike_src):
    mk = os.path.join(spike_src, "riscv", "riscv.mk.in")
    groups, current = {}, None
    with open(mk) as f:
        for line in f:
            m = re.match(r"^(riscv_insn_ext_v_\w+)\s*=", line)
            if m:
                current = m.group(1)
                groups[current] = []
                continue
            if current:
                tok = line.strip().rstrip("\\").strip()
                if not tok:
                    current = None
                elif not tok.startswith("$("):
                    groups[current].append(tok)
    return groups


def normalize(mnemonic):
    name = mnemonic.replace(".", "_")
    m = SEGMENT_RE.match(name)
    if m:
        # vlseg2e32_v -> vle32_v, vlsseg3e8_v -> vlse8_v, vluxseg2ei16_v -> vluxei16_v
        name = re.sub(r"seg\d", "", name)
    return name


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--spike-src", default=os.path.expanduser("~/Documents/Mestrado/riscv-isa-sim"))
    ap.add_argument("--csv", help="write per-instruction coverage to this CSV")
    ap.add_argument("--exclude", default="", help="comma-separated instructions left out of pair/triple coverage")
    ap.add_argument("--pairs-csv", help="write observed instruction pairs (with counts) to this CSV")
    ap.add_argument("--triples-csv", help="write observed instruction triples (with counts) to this CSV")
    args = ap.parse_args()
    exclude = {normalize(i.strip()) for i in args.exclude.split(",") if i.strip()}

    groups = load_reference(args.spike_src)
    reference = {i: g for g, insns in groups.items() for i in insns}

    counts = collections.Counter()
    configs = collections.defaultdict(collections.Counter)  # insn -> {(sew, lmul): n}
    pairs = collections.Counter()    # (a, b): n
    triples = collections.Counter()  # (a, b, c): n
    window = collections.deque(maxlen=3)
    last = None
    with open(args.log, errors="replace") as f:
        for line in f:
            m = COMMIT_RE.match(line)
            if m and last:
                configs[last][(int(m.group(1)), m.group(2))] += 1
                continue
            m = DISASM_RE.match(line)
            if m:
                name = normalize(m.group(1))
                last = name if name in reference else None
                if last:
                    counts[last] += 1
                    if last not in exclude:
                        window.append(last)
                        if len(window) >= 2:
                            pairs[tuple(window)[-2:]] += 1
                        if len(window) == 3:
                            triples[tuple(window)] += 1

    print(f"{'group':28s} {'covered':>8s} {'total':>6s} {'%':>7s}")
    for g, insns in groups.items():
        hit = sum(1 for i in insns if counts[i])
        print(f"{g:28s} {hit:8d} {len(insns):6d} {100 * hit / max(len(insns), 1):6.1f}%")
    hit = sum(1 for i in reference if counts[i])
    print(f"{'TOTAL':28s} {hit:8d} {len(reference):6d} {100 * hit / len(reference):6.1f}%\n")

    covered = sorted(i for i in counts if i not in exclude)
    n = len(covered)
    # per instruction: distinct successors (pairs a->*) and distinct (b, c) suffixes (triples a->*->*)
    pair_first = collections.Counter(a for a, _ in pairs)
    triple_first = collections.Counter(a for a, _, _ in triples)
    print(f"Pair/triple coverage over the {n} covered instructions"
          + (f" (excluding {', '.join(sorted(exclude))})" if exclude else "") + ":")
    print(f"  pairs   {len(pairs):10d} / {n ** 2:12d}  {100 * len(pairs) / max(n ** 2, 1):8.4f}%"
          f"  ({sum(pairs.values())} occurrences)")
    print(f"  triples {len(triples):10d} / {n ** 3:12d}  {100 * len(triples) / max(n ** 3, 1):8.4f}%"
          f"  ({sum(triples.values())} occurrences)\n")

    print(f"Executed instructions (count, pairs as first /{n}, triples as first /{n ** 2}, SEW/LMUL bins):")
    for insn, c in counts.most_common():
        bins = ", ".join(f"e{s}/{l}" for s, l in sorted(configs[insn]))
        print(f"  {insn:20s} {c:12d} {pair_first[insn]:6d} {triple_first[insn]:8d}  {bins}")

    if args.csv:
        with open(args.csv, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["group", "instruction", "count", "pairs_as_first", "triples_as_first", "sew_lmul_bins"])
            for insn, g in reference.items():
                bins = ";".join(f"e{s}/{l}" for s, l in sorted(configs[insn]))
                w.writerow([g, insn, counts[insn], pair_first[insn], triple_first[insn], bins])

    for path, grams, cols in ((args.pairs_csv, pairs, ["first", "second"]),
                              (args.triples_csv, triples, ["first", "second", "third"])):
        if path:
            with open(path, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(cols + ["count"])
                for gram, c in grams.most_common():
                    w.writerow(list(gram) + [c])


if __name__ == "__main__":
    sys.exit(main())
