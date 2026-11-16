#!/usr/bin/env python3
"""
Coverage das 97 instruções suportadas (COVERED_INSTRUCTIONS) em cada um dos 7 testes
kernels do run_kernels.sh: instruções, pares e triplas.

Cada kernel roda no Spike com log de commits; o log é lido com o mesmo parser do
rvv_coverage.py (DISASM_RE + normalize). Pares/triplas são de instruções consecutivas
dentre as 97 (escalares e outras vetoriais no meio, como vle32.v/vse32.v/vsetvli, são
ignoradas), sobre todas as 97^2 / 97^3 combinações.

O tempo de execução é medido numa rodada separada, sem log (como no run_kernels.sh),
com os kernels rodando um de cada vez.

Instruções retiradas: cada kernel também roda com `spike -g` (histograma de PCs, sem log); a
soma do histograma é o total de instruções retiradas (escalares inclusive). Deve bater com a
contagem de linhas de instrução do log ("instr. executadas").

Saídas:  ./test_runs/kernels/coverage/<kernel>.log    (log do Spike; apagado com --no-keep-logs)
         ./test_runs/kernels/coverage/coverage.csv    (instrução x kernel, nº de execuções)
         ./test_runs/kernels/coverage/pairs.csv       (par x kernel, nº de ocorrências)
         ./test_runs/kernels/coverage/triples.csv     (tripla x kernel, nº de ocorrências)

Uso:
    python3 test_runs/kernels_coverage.py [-k lmul,vlen] [--reuse-logs] [--no-timing]
"""

import argparse
import collections
import csv
import os
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from multi_run_all_instructions import COVERED_INSTRUCTIONS, ISA  # noqa: E402
from multi_run_kernels import KERNELS  # noqa: E402
from rvv_coverage import DISASM_RE, normalize  # noqa: E402

DEFAULT_OUT = Path("./test_runs/kernels/coverage")
SUPPORTED = sorted(COVERED_INSTRUCTIONS)
N = len(SUPPORTED)


def spike_cmd(spike, kernel, log=None):
    cmd = [spike]
    if log:
        cmd += ["-l", "--log-commits", f"--log={log}"]
    return cmd + [f"--isa={ISA}", f"./benchmarks/test_{kernel}.riscv"]


def run(cmd, timeout):
    start = time.perf_counter()
    try:
        r = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                           timeout=timeout)
        rc = r.returncode
    except subprocess.TimeoutExpired:
        rc = "timeout"
    return rc, time.perf_counter() - start


def retired(spike, kernel, timeout):
    """Instruções retiradas (soma do histograma do spike -g), ou None em timeout."""
    cmd = [spike, "-g", f"--isa={ISA}", f"./benchmarks/test_{kernel}.riscv"]
    try:
        r = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None
    total = 0
    for line in r.stderr.decode(errors="replace").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[1].isdigit():
            total += int(parts[1])
    return total


def analyze(log):
    """Retorna (instruções executadas no total, contagem por instrução, pares, triplas)."""
    counts, pairs, triples = collections.Counter(), collections.Counter(), collections.Counter()
    window = collections.deque(maxlen=3)
    executed = 0
    with open(log, errors="replace") as f:
        for line in f:
            m = DISASM_RE.match(line)
            if not m:
                continue
            executed += 1
            name = normalize(m.group(1))
            if name not in COVERED_INSTRUCTIONS:
                continue
            counts[name] += 1
            window.append(name)
            if len(window) >= 2:
                pairs[tuple(window)[-2:]] += 1
            if len(window) == 3:
                triples[tuple(window)] += 1
    return executed, counts, pairs, triples


def pct(hit, total):
    return f"{hit:>8d}/{total:<9d} {100 * hit / total:8.4f}%"


def write_grams(path, kernels, grams_by_kernel, cols):
    keys = sorted(set().union(*(grams_by_kernel[k] for k in kernels)))
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(cols + kernels + ["total", "kernels_covering"])
        for g in keys:
            c = [grams_by_kernel[k][g] for k in kernels]
            w.writerow(list(g) + c + [sum(c), sum(1 for x in c if x)])


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-k", "--kernels", default=",".join(KERNELS),
                        help="lista separada por vírgula (padrão: os 7 kernels)")
    parser.add_argument("--spike", default="spike")
    parser.add_argument("--timeout", type=float, default=120, help="segundos por kernel")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 1,
                        help="execuções com log em paralelo")
    parser.add_argument("-o", "--out-dir", help=f"pasta de saída (padrão: {DEFAULT_OUT})")
    parser.add_argument("--reuse-logs", action="store_true",
                        help="não roda o Spike com log, só reanalisa os logs existentes")
    parser.add_argument("--no-timing", action="store_true",
                        help="não mede o tempo de execução (rodada sem log)")
    parser.add_argument("--no-keep-logs", action="store_true", help="apaga os logs no final")
    args = parser.parse_args()

    out_dir = Path(args.out_dir).resolve() if args.out_dir else None
    if Path.cwd().name == "test_runs":
        os.chdir("..")
    out_dir = out_dir or DEFAULT_OUT
    out_dir.mkdir(parents=True, exist_ok=True)

    kernels = [k.strip().removeprefix("test_") for k in args.kernels.split(",") if k.strip()]
    for kernel in kernels:
        if not Path(f"./benchmarks/test_{kernel}.riscv").is_file():
            sys.exit(f"Erro: ./benchmarks/test_{kernel}.riscv não encontrado.")

    # Tempo: uma execução de cada kernel, sem log, um de cada vez
    times = {}
    if not args.no_timing:
        print(f"Medindo tempo (Spike sem log, {ISA})...")
        for k in kernels:
            rc, secs = run(spike_cmd(args.spike, k), args.timeout)
            times[k] = secs
            if rc != 0:
                print(f"  aviso: {k} terminou com {rc}")

    print("Contando instruções retiradas (spike -g)...")
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        retired_n = dict(zip(kernels, ex.map(lambda k: retired(args.spike, k, args.timeout),
                                             kernels)))

    logs = {k: out_dir / f"{k}.log" for k in kernels}
    if not args.reuse_logs:
        print(f"Rodando {len(kernels)} kernels no Spike com log...")
        with ThreadPoolExecutor(max_workers=args.jobs) as ex:
            res = dict(zip(kernels, ex.map(
                lambda k: run(spike_cmd(args.spike, k, logs[k]), args.timeout), kernels)))
        for k, (rc, _) in res.items():
            if rc != 0:
                print(f"  aviso: {k} terminou com {rc} (coverage parcial do log)")

    executed, cov, pairs, triples = {}, {}, {}, {}
    for k in kernels:
        executed[k], cov[k], pairs[k], triples[k] = analyze(logs[k])
    if args.no_keep_logs:
        for log in logs.values():
            log.unlink(missing_ok=True)

    # ---- Tabela instrução x kernel
    short = {k: k[:10] for k in kernels}
    print(f"\n{'instrução':16s}" + "".join(f"{short[k]:>11s}" for k in kernels)
          + f"{'kernels':>9s}")
    for insn in SUPPORTED:
        cells = "".join(f"{('X' if cov[k][insn] else '.'):>11s}" for k in kernels)
        print(f"{insn:16s}{cells}{sum(1 for k in kernels if cov[k][insn]):>9d}")

    # ---- Resumo: instruções, pares, triplas, tempo
    total_insns = {i for k in kernels for i in cov[k]}
    total_pairs = set().union(*(pairs[k] for k in kernels))
    total_triples = set().union(*(triples[k] for k in kernels))

    print(f"\nCoverage sobre as {N} instruções suportadas ({N}^2 = {N ** 2} pares, "
          f"{N}^3 = {N ** 3} triplas):\n")
    hdr = (f"{'kernel':16s} {'instruções':>27s} {'pares':>27s} {'triplas':>27s}"
           f" {'instr. executadas':>18s} {'retiradas (-g)':>15s}" + (f" {'tempo (s)':>10s}" if times else ""))
    print(hdr)
    print("-" * len(hdr))
    for k in kernels:
        line = (f"{k:16s} {pct(len(cov[k]), N)} {pct(len(pairs[k]), N ** 2)}"
                f" {pct(len(triples[k]), N ** 3)} {executed[k]:>18d}"
                f" {('-' if retired_n[k] is None else retired_n[k]):>15}")
        if times:
            line += f" {times[k]:>10.3f}"
        print(line)
    print("-" * len(hdr))
    line = (f"{'TOTAL':16s} {pct(len(total_insns), N)} {pct(len(total_pairs), N ** 2)}"
            f" {pct(len(total_triples), N ** 3)} {sum(executed.values()):>18d}"
            f" {sum(v or 0 for v in retired_n.values()):>15d}")
    if times:
        line += f" {sum(times.values()):>10.3f}"
    print(line)
    print("  (TOTAL = união do que os kernels cobrem juntos; tempo e instruções = soma)")

    missing = [i for i in SUPPORTED if i not in total_insns]
    print(f"\nInstruções não cobertas por nenhum kernel ({len(missing)}):")
    print("  " + (", ".join(missing) if missing else "-"))

    print(f"\nPares cobertos ({len(total_pairs)}):")
    for a, b in sorted(total_pairs):
        ks = [short[k] for k in kernels if pairs[k][(a, b)]]
        print(f"  {a:14s} -> {b:14s}  {', '.join(ks)}")

    # ---- CSVs
    with open(out_dir / "coverage.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["instruction"] + kernels + ["total", "kernels_covering"])
        for insn in SUPPORTED:
            c = [cov[k][insn] for k in kernels]
            w.writerow([insn] + c + [sum(c), sum(1 for x in c if x)])
    write_grams(out_dir / "pairs.csv", kernels, pairs, ["first", "second"])
    write_grams(out_dir / "triples.csv", kernels, triples, ["first", "second", "third"])
    print(f"\nCSVs: {out_dir}/coverage.csv, pairs.csv, triples.csv")


if __name__ == "__main__":
    sys.exit(main())
