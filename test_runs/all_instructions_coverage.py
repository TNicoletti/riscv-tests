#!/usr/bin/env python3
"""
Coverage das 97 instruções suportadas (COVERED_INSTRUCTIONS) no test_all_instructions:
instruções, pares e triplas, no mesmo formato do kernels_coverage.py.

O binário é preparado como no multi_run_all_instructions.py: params.json (mem_setter) na
.PARAMETERS_SECTION e um arquivo de entrada aleatório na .OUT_SECTION (padrão 153.int32.in).
O Spike roda com -l --log-commits e o log é lido direto do pipe (nada vai para o disco),
com o mesmo parser do rvv_coverage.py. Pares/triplas são de instruções consecutivas dentre as
97 (escalares e as outras vetoriais no meio — vle32.v, vse32.v, vsetvli e as fora das 97 —
são ignoradas), sobre todas as 97^2 / 97^3 combinações.

O tempo de execução é medido numa rodada separada, sem log. As instruções retiradas
(escalares inclusive) vêm de outra rodada com `spike -g` (soma do histograma de PCs).

Saídas:  ./benchmarks/test_all_instructions/test_runs/coverage/coverage.csv  (instrução, nº de execuções)
         .../pairs.csv, .../triples.csv, .../report.txt

Uso:
    python3 test_runs/all_instructions_coverage.py [--input ARQ] [--params JSON]
"""

import argparse
import collections
import csv
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from multi_run_all_instructions import (BENCH, COVERED_INSTRUCTIONS, ISA,  # noqa: E402
                                        prepare_binary)
from rvv_coverage import DISASM_RE, normalize  # noqa: E402

DEFAULT_OUT = Path(f"./benchmarks/{BENCH}/test_runs/coverage")
SUPPORTED = sorted(COVERED_INSTRUCTIONS)
N = len(SUPPORTED)


def run_with_log(spike, riscv, timeout):
    """Roda com log em pipe; devolve (rc, executadas, vetoriais fora das 97, contagens, pares, triplas)."""
    counts, others = collections.Counter(), collections.Counter()
    pairs, triples = collections.Counter(), collections.Counter()
    window = collections.deque(maxlen=3)
    executed = 0
    proc = subprocess.Popen([spike, "-l", "--log-commits", f"--isa={ISA}", str(riscv)],
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    start = time.monotonic()
    try:
        for raw in proc.stderr:
            m = DISASM_RE.match(raw.decode("ascii", errors="replace"))
            if not m:
                continue
            executed += 1
            name = normalize(m.group(1))
            if name not in COVERED_INSTRUCTIONS:
                if name.startswith("v"):
                    others[name] += 1
                continue
            counts[name] += 1
            window.append(name)
            if len(window) >= 2:
                pairs[tuple(window)[-2:]] += 1
            if len(window) == 3:
                triples[tuple(window)] += 1
            if time.monotonic() - start > timeout:
                raise subprocess.TimeoutExpired(proc.args, timeout)
        rc = proc.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
        rc = "timeout"
    return rc, executed, others, counts, pairs, triples


def pct(hit, total):
    return f"{hit:>8d}/{total:<9d} {100 * hit / total:8.4f}%"


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--input", default="./test_runs/uniform/inputs32/0153.int32.in",
                        help="valores iniciais (.OUT_SECTION)")
    parser.add_argument("--params", default=f"./benchmarks/{BENCH}/params.json")
    parser.add_argument("--spike", default="spike")
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("-o", "--out-dir", help=f"pasta de saída (padrão: {DEFAULT_OUT})")
    args = parser.parse_args()

    cwd = Path.cwd()
    input_file = (cwd / args.input).resolve() if args.input != parser.get_default("input") else None
    params = (cwd / args.params).resolve() if args.params != parser.get_default("params") else None
    out_dir = (cwd / args.out_dir).resolve() if args.out_dir else None
    if cwd.name == "test_runs":
        os.chdir("..")
    input_file = input_file or Path(args.input).resolve()
    params = params or Path(args.params).resolve()
    out_dir = out_dir or DEFAULT_OUT
    out_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="taicov_") as tmp:
        riscv = prepare_binary(params, input_file, Path(tmp))

        start = time.perf_counter()
        r = subprocess.run([args.spike, f"--isa={ISA}", str(riscv)], stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL, timeout=args.timeout)
        plain_seconds = time.perf_counter() - start
        if r.returncode != 0:
            print(f"aviso: execução sem log terminou com {r.returncode}")

        retired = None
        try:
            g = subprocess.run([args.spike, "-g", f"--isa={ISA}", str(riscv)],
                               stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                               timeout=args.timeout)
            retired = sum(int(parts[1]) for parts in
                          (l.split() for l in g.stderr.decode(errors="replace").splitlines())
                          if len(parts) == 2 and parts[1].isdigit())
        except subprocess.TimeoutExpired:
            print("aviso: spike -g deu timeout; instruções retiradas não contadas")

        start = time.perf_counter()
        rc, executed, others, cov, pairs, triples = run_with_log(args.spike, riscv, args.timeout)
        log_seconds = time.perf_counter() - start
        if rc != 0:
            print(f"aviso: execução com log terminou com {rc} (coverage parcial)")

    lines = [f"test_all_instructions — entrada {input_file.name}, parâmetros {params}", ""]
    lines.append(f"{'instrução':16s} {'execuções':>10s}")
    for insn in SUPPORTED:
        lines.append(f"{insn:16s} {cov[insn]:>10d}")
    lines.append("")
    lines.append(f"Coverage sobre as {N} instruções suportadas ({N}^2 = {N ** 2} pares, "
                 f"{N}^3 = {N ** 3} triplas):")
    lines.append(f"  instruções {pct(len(cov), N)}")
    lines.append(f"  pares      {pct(len(pairs), N ** 2)}  ({sum(pairs.values())} ocorrências)")
    lines.append(f"  triplas    {pct(len(triples), N ** 3)}  ({sum(triples.values())} ocorrências)")
    lines.append(f"  instruções executadas (log, escalares inclusive): {executed}")
    if retired is not None:
        lines.append(f"  instruções retiradas (spike -g, escalares inclusive): {retired}")
        lines.append(f"  vetoriais das 97: {sum(cov.values())} "
                     f"({100 * sum(cov.values()) / retired:.2f}% das retiradas)")
    lines.append(f"  tempo sem log: {plain_seconds:.3f} s   (com log, só para referência: "
                 f"{log_seconds:.1f} s)")
    missing = [i for i in SUPPORTED if not cov[i]]
    lines.append("")
    lines.append(f"Instruções suportadas não executadas ({len(missing)}): "
                 + (", ".join(missing) if missing else "-"))
    extra = sorted(i for i in others if i not in ("vle32_v", "vse32_v", "vsetvli"))
    lines.append(f"Vetoriais executadas fora das 97 ({len(extra)}): "
                 + (", ".join(extra) if extra else "-"))
    report = "\n".join(lines) + "\n"
    print(report)
    (out_dir / "report.txt").write_text(report)

    with open(out_dir / "coverage.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["instruction", "count"])
        w.writerows([insn, cov[insn]] for insn in SUPPORTED)
    for name, grams, cols in (("pairs.csv", pairs, ["first", "second"]),
                              ("triples.csv", triples, ["first", "second", "third"])):
        with open(out_dir / name, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(cols + ["count"])
            for g, c in grams.most_common():
                w.writerow(list(g) + [c])
    print(f"Saídas em {out_dir}: report.txt, coverage.csv, pairs.csv, triples.csv")


if __name__ == "__main__":
    sys.exit(main())
