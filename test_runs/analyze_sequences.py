#!/usr/bin/env python3
"""
Uma tabela por modelo (M1..M6, M5 separado em triplas e quádruplas): para cada falha e seed,
quantas sequências foram executadas até a seed parar.

    python3 test_runs/analyze_sequences.py [PAPER_DIR] [-e M1,M5] [--export]

Célula:
    N        a seed detectou na sequência N (arquivos anteriores x 1332 + teste da divergência + 1)
    >N       a seed não detectou e parou depois de rodar todos os arquivos (N = arquivos x 1332)
    ?        detectou, mas sem a linha "Divergence" para saber o teste exato

No fim de cada tabela: média, mediana, mínimo e máximo por seed (só seeds que detectaram).
Com --export grava PAPER/sequences.csv (experimento, modelo, falha, seed, detectou, sequências).
"""

import argparse
import csv
import statistics
import sys
from pathlib import Path

from analyze_summary import MODELS
from paper_analysis import DEFAULT_PAPER_DIR, TESTS_PER_FILE, analyze_experiment


def cell(r):
    if r is None:
        return ""
    if not r.detected:
        return f">{r.files_run * TESTS_PER_FILE}"
    return "?" if r.sequences is None else str(r.sequences)


def stats_row(values):
    if not values:
        return ["-"] * 4
    return [f"{statistics.mean(values):.0f}", f"{statistics.median(values):.0f}",
            str(min(values)), str(max(values))]


def print_table(name, results):
    seeds = sorted({r.seed for r in results})
    faults = list(dict.fromkeys(r.fault for r in results))
    by_key = {(r.fault, r.seed): r for r in results}
    width = max(14, max(len(f) for f in faults))
    col = max(9, max(len(cell(r)) for r in results))
    line = f"{'falha':<{width}}  " + "  ".join(f"{s:>{col}}" for s in seeds) + f"  {'todas':>{col}}"

    print("=" * len(line))
    print(f" {name} — sequências executadas até a seed parar")
    print("=" * len(line))
    print(line)
    print("-" * len(line))
    for fault in faults:
        rs = [by_key.get((fault, s)) for s in seeds]
        found = [r.sequences for r in rs if r and r.detected and r.sequences is not None]
        mean = f"{statistics.mean(found):.0f}" if found else "-"
        print(f"{fault:<{width}}  " + "  ".join(f"{cell(r):>{col}}" for r in rs) + f"  {mean:>{col}}")
    print("-" * len(line))

    per_seed = {s: [by_key[(f, s)].sequences for f in faults if (f, s) in by_key
                    and by_key[(f, s)].detected and by_key[(f, s)].sequences is not None]
                for s in seeds}
    everything = [v for vs in per_seed.values() for v in vs]
    columns = [stats_row(per_seed[s]) for s in seeds] + [stats_row(everything)]
    for i, label in enumerate(("média", "mediana", "mínimo", "máximo")):
        print(f"{label:<{width}}  " + "  ".join(f"{c[i]:>{col}}" for c in columns))
    detected = [f"{len(per_seed[s])}/{sum((f, s) in by_key for f in faults)}" for s in seeds]
    detected.append(f"{len(everything)}/{len(results)}")
    print(f"{'detectou':<{width}}  " + "  ".join(f"{d:>{col}}" for d in detected))
    print("  >N = não detectou (rodou todos os arquivos); estatísticas só com as seeds que detectaram;")
    print("  coluna 'todas' = média das seeds da falha / estatística de todas as seeds juntas")


def main():
    parser = argparse.ArgumentParser(description="Sequências até cada seed parar, por modelo.")
    parser.add_argument("paper_dir", nargs="?", default=str(DEFAULT_PAPER_DIR),
                        help=f"pasta de resultados do multi_run_paper.py (padrão: {DEFAULT_PAPER_DIR})")
    parser.add_argument("-e", "--experiments", help="ex.: M1,M5 (padrão: todos)")
    parser.add_argument("--export", action="store_true", help="grava PAPER/sequences.csv")
    args = parser.parse_args()

    paper_dir = Path(args.paper_dir)
    if not paper_dir.is_dir() and Path.cwd().name == "test_runs":
        paper_dir = Path("..") / paper_dir
    if not paper_dir.is_dir():
        sys.exit(f"Erro: '{paper_dir}' não encontrado.")
    wanted = {e.strip().upper() for e in args.experiments.split(",")} if args.experiments else None

    rows = []
    first = True
    for model in MODELS:
        if wanted and model.EXPERIMENT not in wanted:
            continue
        results = analyze_experiment(paper_dir, model.EXPERIMENT, model.minimized, model.labeled)
        groups = model.groups(results) if hasattr(model, "groups") else [(model.TITLE, results)]
        for title, group in groups:
            if not group:
                continue
            if not first:
                print("\n")
            first = False
            print_table(f"{model.EXPERIMENT} {title}", group)
            rows += [[model.EXPERIMENT, title, r.fault, r.seed, int(r.detected), r.files_run,
                      "" if r.sequences is None else r.sequences] for r in group]

    if first:
        sys.exit(f"Nenhum experimento encontrado em '{paper_dir}'.")
    if args.export:
        path = paper_dir / "sequences.csv"
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["experiment", "model", "fault", "seed", "detected", "files_run", "sequences"])
            w.writerows(rows)
        print(f"\nCSV exportado para: {path}")


if __name__ == "__main__":
    main()
