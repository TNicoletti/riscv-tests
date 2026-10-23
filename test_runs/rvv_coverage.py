#!/usr/bin/env python3
"""RVV instruction coverage from a Spike commit log.

Usage:
  spike -l --log-commits --log=run.log --isa=rv64gcv_zvl128b_zicntr_zba_zbb prog.riscv
  python3 test_runs/rvv_coverage.py run.log [--spike-src ~/Documents/Mestrado/riscv-isa-sim] [--csv out.csv]

Reports:
  - which RVV instructions (from Spike's own riscv_insn_ext_v list) were executed
  - which (instruction, SEW, LMUL) combinations were exercised
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
    args = ap.parse_args()

    groups = load_reference(args.spike_src)
    reference = {i: g for g, insns in groups.items() for i in insns}

    counts = collections.Counter()
    configs = collections.defaultdict(collections.Counter)  # insn -> {(sew, lmul): n}
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

    print(f"{'group':28s} {'covered':>8s} {'total':>6s} {'%':>7s}")
    for g, insns in groups.items():
        hit = sum(1 for i in insns if counts[i])
        print(f"{g:28s} {hit:8d} {len(insns):6d} {100 * hit / max(len(insns), 1):6.1f}%")
    hit = sum(1 for i in reference if counts[i])
    print(f"{'TOTAL':28s} {hit:8d} {len(reference):6d} {100 * hit / len(reference):6.1f}%\n")

    print("Executed instructions (count, SEW/LMUL bins):")
    for insn, n in counts.most_common():
        bins = ", ".join(f"e{s}/{l}" for s, l in sorted(configs[insn]))
        print(f"  {insn:20s} {n:12d}  {bins}")

    if args.csv:
        with open(args.csv, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["group", "instruction", "count", "sew_lmul_bins"])
            for insn, g in reference.items():
                bins = ";".join(f"e{s}/{l}" for s, l in sorted(configs[insn]))
                w.writerow([g, insn, counts[insn], bins])


if __name__ == "__main__":
    sys.exit(main())
