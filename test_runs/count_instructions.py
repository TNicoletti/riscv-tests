#!/usr/bin/env python3
"""
Quantas instruções (todas, escalares inclusive) foram executadas ao todo em cada experimento
M1..M6 (e no golden) de uma rodada do multi_run_paper.py.

Refaz cada execução listada em PAPER/<M>/manifest.csv com o mesmo Spike, a mesma falha e a
mesma entrada, adicionando só -g (histograma de PCs, impresso no stderr ao sair). A soma do
histograma é o número de instruções executadas. Não gera log de instruções; fica ~2x mais lento
que sem -g. A saída (stdout) é comparada com o .out original para garantir que a execução foi
a mesma; execuções que deram timeout na rodada original não são contadas (o Spike é morto
antes de imprimir o histograma).

    python3 test_runs/count_instructions.py [PAPER_DIR] [-e M1,M5] [-j 16]
    python3 test_runs/count_instructions.py [PAPER_DIR] --report-only   # refaz o resumo do CSV

M5 aparece separado em triplas (three*) e quádruplas (four*).

Saídas: PAPER/instructions.csv  (uma linha por execução)
        PAPER/instructions.txt  (resumo por experimento)
"""

import argparse
import csv
import os
import shutil
import subprocess
import sys
import tempfile
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from multi_run_paper import ISA, prepare_binaries  # noqa: E402
from paper_analysis import DEFAULT_PAPER_DIR  # noqa: E402

EXPERIMENTS = ["golden", "M1", "M2", "M3", "M4", "M5", "M6"]


def run(spike, spike_args, riscv, expected_out, timeout):
    """(instruções ou None, saída igual à original?, status)"""
    cmd = [spike, "-g", f"--isa={ISA}", *spike_args.split(), str(riscv)]
    try:
        r = subprocess.run(cmd, capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None, None, "timeout"
    total = 0
    for line in r.stderr.decode(errors="replace").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[1].isdigit():
            total += int(parts[1])
    same = None
    if expected_out and Path(expected_out).exists():
        same = r.stdout == Path(expected_out).read_bytes()
    return total, same, str(r.returncode)


def group(experiment, fault):
    """Linha do resumo: M5 separado em triplas e quádruplas."""
    if experiment == "M5":
        return "M5 triplas" if fault.startswith("three") else "M5 quádruplas"
    return experiment


def dots(n):
    return f"{round(n):,}".replace(",", ".")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("paper_dir", nargs="?", default=str(DEFAULT_PAPER_DIR))
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS))
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 1)
    parser.add_argument("--timeout", type=float, default=180, help="segundos por execução")
    parser.add_argument("--spike", default="spike")
    parser.add_argument("--bits", default="32")
    parser.add_argument("--dist", default="uniform")
    parser.add_argument("--report-only", action="store_true",
                        help="não roda o Spike; refaz o resumo a partir de PAPER/instructions.csv")
    args = parser.parse_args()

    paper = Path(args.paper_dir).resolve()
    if Path.cwd().name == "test_runs":
        os.chdir("..")
    if not paper.is_dir():
        sys.exit(f"Erro: '{paper}' não encontrado.")
    experiments = [e.strip() for e in args.experiments.split(",") if e.strip()]
    experiments = [e if e == "golden" else e.upper() for e in experiments]

    if args.report_only:
        with open(paper / "instructions.csv", newline="") as f:
            data = [r for r in csv.DictReader(f) if r["experiment"] in experiments]
        rows = data
        results = [(int(r["instructions"]) if r["instructions"] else None,
                    None if r["same_output"] == "" else r["same_output"] == "1", r["status"])
                   for r in data]
        report(paper, experiments, rows, results)
        return

    rows = []
    for experiment in experiments:
        manifest = paper / experiment / "manifest.csv"
        if manifest.exists():
            with open(manifest, newline="") as f:
                rows += list(csv.DictReader(f))
    if not rows:
        sys.exit(f"Nenhum manifest.csv em '{paper}'.")

    input_dir = Path(f"./test_runs/{args.dist}/inputs{args.bits}")
    inputs = sorted({input_dir / r["input"] for r in rows})
    print(f"{len(rows)} execuções, {len(inputs)} entradas")

    tmp = Path(tempfile.mkdtemp(prefix="count_insns_"))
    try:
        binaries = prepare_binaries("random_neural_network", inputs, tmp)

        def job(r):
            if r["return_code"] == "timeout":
                return None, None, "timeout (original)"
            return run(args.spike, r["spike_args"], binaries[input_dir / r["input"]],
                       r["output"], args.timeout)

        results = []
        with ThreadPoolExecutor(max_workers=args.jobs) as ex:
            for i, res in enumerate(ex.map(job, rows), 1):
                results.append(res)
                if i % 200 == 0:
                    print(f"[{i}/{len(rows)}]")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    with open(paper / "instructions.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["experiment", "fault", "seed", "input", "instructions", "same_output", "status"])
        for r, (n, same, status) in zip(rows, results):
            w.writerow([r["experiment"], r["fault"], r["seed"], r["input"],
                        "" if n is None else n, "" if same is None else int(same), status])
    report(paper, experiments, rows, results)


def report(paper, experiments, rows, results):
    summary = defaultdict(lambda: {"runs": 0, "counted": 0, "insns": 0, "faults": set(),
                                   "skipped": 0, "diff": 0})
    for r, (n, same, status) in zip(rows, results):
        s = summary[group(r["experiment"], r["fault"])]
        s["runs"] += 1
        s["faults"].add(r["fault"])
        if n is None:
            s["skipped"] += 1
            continue
        s["counted"] += 1
        s["insns"] += n
        s["diff"] += same is False

    lines = [f"Instruções executadas (todas, escalares inclusive) — {paper}", "",
             f"{'exp.':<14} {'falhas':>6} {'execuções':>9} {'instruções':>16} "
             f"{'por falha':>14} {'por execução':>13} {'sem contar':>10} {'saída ≠':>8}"]
    total = 0
    order = [g for e in experiments
             for g in (["M5 triplas", "M5 quádruplas"] if e == "M5" else [e])]
    for e in order:
        if e not in summary:
            continue
        s = summary[e]
        total += s["insns"] if e != "golden" else 0
        per_fault = s["insns"] / len(s["faults"])
        per_run = s["insns"] / s["counted"] if s["counted"] else 0
        lines.append(f"{e:<14} {len(s['faults']):>6} {s['runs']:>9} {dots(s['insns']):>16} "
                     f"{dots(per_fault):>14} {dots(per_run):>13} {s['skipped']:>10} {s['diff']:>8}")
    lines.append(f"{'M1..M6':<14} {'':>6} {'':>9} {dots(total):>16}")
    lines += ["",
              "sem contar = execuções que deram timeout (o Spike é morto antes de imprimir o histograma)",
              "saída ≠    = a reexecução com -g não reproduziu o .out original"]
    report = "\n".join(lines) + "\n"
    print("\n" + report)
    (paper / "instructions.txt").write_text(report)
    print(f"Detalhes em {paper / 'instructions.csv'}")


if __name__ == "__main__":
    main()
