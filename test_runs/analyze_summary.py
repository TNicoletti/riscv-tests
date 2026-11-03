#!/usr/bin/env python3
"""
Junta as análises de M1..M6 (analyze_m1.py .. analyze_m6.py) numa visão geral, no
formato das Tabelas III e IV do artigo.

    python3 test_runs/analyze_summary.py [PAPER_DIR] [--export]

Mostra:
    - por modelo e seed: quantas falhas foram detectadas, minimizadas e rotuladas
    - por modelo: quantas falhas alguma seed detectou/minimizou/rotulou
    - custo: média de sequências executadas até detectar e tempo médio por seed
    - falsos positivos no Spike padrão (golden)
    - falhas que nenhuma seed detectou

Com --export grava PAPER/summary.csv (uma linha por modelo e seed).
"""

import argparse
import csv
import sys
from pathlib import Path

import analyze_m1
import analyze_m2
import analyze_m3
import analyze_m4
import analyze_m5
import analyze_m6
from paper_analysis import (DEFAULT_PAPER_DIR, analyze_experiment, cost, cost_summary, fmt,
                            parse_output, pct, seed_summary)

MODELS = [analyze_m1, analyze_m2, analyze_m3, analyze_m4, analyze_m5, analyze_m6]


def golden_false_positives(paper_dir):
    """{seed: (arquivos, arquivos que detectaram)} do Spike padrão."""
    result = {}
    golden = paper_dir / "golden" / "golden"
    if not golden.is_dir():
        return result
    for seed_dir in sorted(p for p in golden.iterdir() if p.is_dir()):
        files = sorted(seed_dir.glob("*.out"))
        result[seed_dir.name] = (len(files), sum(parse_output(f).detected for f in files))
    return result


def main():
    parser = argparse.ArgumentParser(description="Resumo dos experimentos M1-M6.")
    parser.add_argument("paper_dir", nargs="?", default=str(DEFAULT_PAPER_DIR),
                        help=f"pasta de resultados do multi_run_paper.py (padrão: {DEFAULT_PAPER_DIR})")
    parser.add_argument("--export", action="store_true", help="grava PAPER/summary.csv")
    args = parser.parse_args()

    paper_dir = Path(args.paper_dir)
    if not paper_dir.is_dir() and Path.cwd().name == "test_runs":
        paper_dir = Path("..") / paper_dir
    if not paper_dir.is_dir():
        sys.exit(f"Erro: '{paper_dir}' não encontrado.")

    # {(experimento, título): results}; modelos com groups() (M5) viram várias linhas
    all_results = {}
    for model in MODELS:
        results = analyze_experiment(paper_dir, model.EXPERIMENT, model.minimized, model.labeled)
        groups = model.groups(results) if hasattr(model, "groups") else [(model.TITLE, results)]
        for title, group_results in groups:
            if group_results:
                all_results[(model.EXPERIMENT, title)] = group_results

    if not all_results:
        sys.exit(f"Nenhum experimento encontrado em '{paper_dir}'.")

    seeds = sorted({r.seed for results in all_results.values() for r in results})
    rows = []

    # ------------------------------------------------------------------
    print("=" * 96)
    print(" POR MODELO E SEED (falhas detectadas / minimizadas / rotuladas certo)")
    print("=" * 96)
    print(f"{'modelo':<27} {'seed':<5} {'N':>4} {'detectou':>16} {'minimizou':>16} {'rotulou':>16} "
          f"{'seq. média':>11} {'tempo médio':>12}")
    for (experiment, title), results in all_results.items():
        name = f"{experiment} {title}"
        costs = cost_summary(results)
        for seed, (t, d, m, l) in seed_summary(results).items():
            mean_seq, median_seq, mean_sec, total_sec = costs[seed]
            print(f"{name:<27} {seed:<5} {t:>4} {pct(d, t):>16} {pct(m, t):>16} {pct(l, t):>16} "
                  f"{fmt(mean_seq, '.0f'):>11} {fmt(mean_sec, '.2f', 's'):>12}")
            rows.append([experiment, title, seed, t, d, m, l,
                         "" if mean_seq is None else f"{mean_seq:.0f}",
                         "" if median_seq is None else median_seq,
                         "" if mean_sec is None else f"{mean_sec:.2f}",
                         "" if total_sec is None else f"{total_sec:.1f}"])
            name = ""
        print()
    print("seq. média = sequências executadas até detectar (seeds que detectaram); "
          "tempo médio = de uma seed\n")

    # ------------------------------------------------------------------
    print("=" * 96)
    print(" POR MODELO (uma falha conta se alguma seed conseguiu)")
    print("=" * 96)
    print(f"{'modelo':<27} {'N':>4} {'detectou':>16} {'minimizou':>16} {'rotulou':>16} {'mean length':>14}")
    totals = [0, 0, 0, 0]
    for (experiment, title), results in all_results.items():
        faults = {}
        for r in results:
            d, m, l = faults.get(r.fault, (False, False, False))
            faults[r.fault] = (d or r.detected, m or r.minimized, l or r.labeled)
        t = len(faults)
        d = sum(v[0] for v in faults.values())
        m = sum(v[1] for v in faults.values())
        l = sum(v[2] for v in faults.values())
        for i, v in enumerate((t, d, m, l)):
            totals[i] += v
        mean_seq = cost(results)[0]
        print(f"{experiment + ' ' + title:<27} {t:>4} {pct(d, t):>16} {pct(m, t):>16} "
              f"{pct(l, t):>16} {fmt(mean_seq, '.0f'):>14}")
        rows.append([experiment, title, "alguma", t, d, m, l,
                     "" if mean_seq is None else f"{mean_seq:.0f}", "", "", ""])
    t, d, m, l = totals
    mean_seq = cost([r for results in all_results.values() for r in results])[0]
    print(f"{'Total':<27} {t:>4} {pct(d, t):>16} {pct(m, t):>16} {pct(l, t):>16} {fmt(mean_seq, '.0f'):>14}")
    rows.append(["Total", "", "alguma", t, d, m, l, "" if mean_seq is None else f"{mean_seq:.0f}", "", "", ""])

    # ------------------------------------------------------------------
    print()
    print("=" * 96)
    print(" POR SEED (todos os modelos)")
    print("=" * 96)
    print(f"{'seed':<6} {'N':>4} {'detectou':>16} {'minimizou':>16} {'rotulou':>16} {'mean length':>14}")
    for seed in seeds:
        sr = [r for results in all_results.values() for r in results if r.seed == seed]
        t = len(sr)
        print(f"{seed:<6} {t:>4} {pct(sum(r.detected for r in sr), t):>16} "
              f"{pct(sum(r.minimized for r in sr), t):>16} {pct(sum(r.labeled for r in sr), t):>16} "
              f"{fmt(cost(sr)[0], '.0f'):>14}")
    print("\nmean length = média de sequências executadas até detectar (todas as seeds que detectaram)")

    # ------------------------------------------------------------------
    golden = golden_false_positives(paper_dir)
    if golden:
        print()
        print("Falsos positivos no Spike padrão (golden): " +
              ", ".join(f"seed {s}: {fp}/{n} arquivos" for s, (n, fp) in golden.items()))

    print()
    print("Falhas que nenhuma seed detectou:")
    any_missing = False
    for (experiment, title), results in all_results.items():
        detected = {r.fault for r in results if r.detected}
        missing = sorted({r.fault for r in results} - detected)
        if missing:
            any_missing = True
            print(f"  {experiment} {title}: {' '.join(missing)}")
    if not any_missing:
        print("  (nenhuma)")

    incomplete = sum(r.detected and not r.finished for results in all_results.values() for r in results)
    if incomplete:
        print(f"\n{incomplete} seed(s) detectaram mas o diagnóstico não terminou (timeout).")

    if args.export:
        path = paper_dir / "summary.csv"
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["experiment", "model", "seed", "faults", "detected", "minimized", "labeled",
                        "mean_sequences", "median_sequences", "mean_seed_seconds",
                        "total_seconds"])
            w.writerows(rows)
        print(f"\nCSV exportado para: {path}")


if __name__ == "__main__":
    main()
