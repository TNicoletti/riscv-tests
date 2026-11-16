#!/usr/bin/env python3
"""
Resumo de quais falhas M1-M6 algum teste kernel (S1) detecta, a partir das saídas do
multi_run_kernels.py. Critérios em kernel_analysis.py:

    detectou  algum kernel travou (crash) ou acusou erro sozinho (check)
    só diff   nenhum kernel acusou, mas algum teve saída diferente do Spike padrão

    python3 test_runs/analyze_kernels.py [KPAPER_DIR] [-e M1,M6] [-k lmul,vlen] [--list] [--export]

--list   por falha, o que cada kernel fez (C = crash, F = check, d = diff, . = ok)
--export grava KPAPER/<M>/analysis.csv (falha x kernel) e KPAPER/summary.csv

Para um kernel só: python3 test_runs/analyze_kernel.py KERNEL
"""

import argparse
import csv
import sys

from kernel_analysis import (DEFAULT_KPAPER_DIR, EXPERIMENTS, KERNELS, SHORT, by_fault,
                             analyze_experiment, groups, load_golden, pct, resolve_dir)

LETTER = {"crash": "C", "check": "F", "diff": "d", "ok": "."}


def main():
    parser = argparse.ArgumentParser(description="Resumo das falhas M1-M6 nos testes kernels.")
    parser.add_argument("kpaper_dir", nargs="?", default=str(DEFAULT_KPAPER_DIR),
                        help=f"pasta de resultados (padrão: {DEFAULT_KPAPER_DIR})")
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS))
    parser.add_argument("-k", "--kernels", default=",".join(KERNELS))
    parser.add_argument("--list", action="store_true", help="mostra cada falha x kernel")
    parser.add_argument("--export", action="store_true",
                        help="grava KPAPER/<M>/analysis.csv e KPAPER/summary.csv")
    args = parser.parse_args()

    kpaper = resolve_dir(args.kpaper_dir)
    if not kpaper.is_dir():
        sys.exit(f"Erro: '{kpaper}' não encontrado.")
    experiments = [e.strip().upper() for e in args.experiments.split(",") if e.strip()]
    kernels = [k.strip().removeprefix("test_") for k in args.kernels.split(",") if k.strip()]
    golden = load_golden(kpaper)
    if not golden:
        print("Aviso: sem saídas do Spike padrão (golden); 'diff' não será calculado.\n")

    table = []      # (nome, faults)
    rows = []
    for experiment in experiments:
        runs = analyze_experiment(kpaper, experiment, golden, kernels)
        if not runs:
            continue
        faults = by_fault(runs)
        for title, names in groups(experiment, faults):
            if names:
                table.append((experiment, title, {f: faults[f] for f in names}))

        if args.list:
            print("=" * 78)
            print(f" {experiment} — C = crash, F = kernel acusou, d = só diferente do golden, . = ok")
            print("=" * 78)
            width = max(12, max(len(f) for f in faults))
            print(f"{'falha':<{width}}  " + " ".join(f"{SHORT[k]:>6}" for k in kernels)
                  + "  onde")
            for fault, by_kernel in faults.items():
                cells = " ".join(f"{LETTER[by_kernel[k].status] if k in by_kernel else ' ':>6}"
                                 for k in kernels)
                where = "; ".join(f"{SHORT[k]}: {r.where}" for k, r in by_kernel.items()
                                  if r.detected and r.where)
                print(f"{fault:<{width}}  {cells}  {where}")
            print()

        if args.export:
            with open(kpaper / experiment / "analysis.csv", "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["fault", "kernel", "status", "detected", "return_code", "where",
                            "seconds"])
                for r in runs:
                    w.writerow([r.fault, r.kernel, r.status, int(r.detected), r.rc, r.where,
                                "" if r.seconds is None else f"{r.seconds:.2f}"])

    if not table:
        sys.exit(f"Nenhum experimento encontrado em '{kpaper}'.")

    # ------------------------------------------------------------------
    print("=" * 110)
    print(" FALHAS DETECTADAS PELOS TESTES KERNELS (S1) — uma execução por falha e kernel")
    print("=" * 110)
    print(f"{'modelo':<27} {'N':>4} {'detectou':>15} {'só diff':>8}  "
          + " ".join(f"{SHORT[k]:>6}" for k in kernels))
    total_n = total_d = total_diff = 0
    for experiment, title, faults in table:
        n = len(faults)
        d = sum(any(r.detected for r in ks.values()) for ks in faults.values())
        only_diff = sum(not any(r.detected for r in ks.values())
                        and any(r.status == "diff" for r in ks.values()) for ks in faults.values())
        per_kernel = [sum(ks[k].detected for ks in faults.values() if k in ks) for k in kernels]
        total_n, total_d, total_diff = total_n + n, total_d + d, total_diff + only_diff
        print(f"{experiment + ' ' + title:<27} {n:>4} {pct(d, n):>15} {only_diff:>8}  "
              + " ".join(f"{c:>6}" for c in per_kernel))
        rows.append([experiment, title, n, d, only_diff, *per_kernel])
    print(f"{'Total':<27} {total_n:>4} {pct(total_d, total_n):>15} {total_diff:>8}")
    rows.append(["Total", "", total_n, total_d, total_diff] + [""] * len(kernels))
    print("  colunas por kernel = falhas que aquele kernel detectou (crash ou check)")

    # Como cada kernel detectou (todas as falhas somadas)
    print()
    print(f"{'kernel':<16} {'crash':>6} {'check':>6} {'diff':>6} {'ok':>6}  {'detectou sozinho':>16}")
    all_faults = [ks for _, _, faults in table for ks in faults.values()]
    for k in kernels:
        statuses = [ks[k].status for ks in all_faults if k in ks]
        alone = sum(ks[k].detected and not any(r.detected for kk, r in ks.items() if kk != k)
                    for ks in all_faults if k in ks)
        print(f"{k:<16} " + " ".join(f"{statuses.count(s):>6}" for s in ("crash", "check", "diff", "ok"))
              + f"  {alone:>16}")
    print("  detectou sozinho = falhas que só esse kernel detectou")

    if golden:
        g = analyze_experiment(kpaper, "golden", {k: v for k, v in golden.items()}, kernels)
        fp = [r.kernel for r in g if r.detected]
        print(f"\nFalsos positivos no Spike padrão (golden): {', '.join(fp) if fp else 'nenhum'}")

    print("\nFalhas que nenhum kernel detectou:")
    for experiment, title, faults in table:
        missing = [f for f, ks in faults.items() if not any(r.detected for r in ks.values())]
        if missing:
            shown = " ".join(missing) if len(missing) <= 30 else " ".join(missing[:30]) + " ..."
            print(f"  {experiment} {title} ({len(missing)}/{len(faults)}): {shown}")

    if args.export:
        with open(kpaper / "summary.csv", "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["experiment", "model", "faults", "detected", "only_diff",
                        *[f"detected_by_{k}" for k in kernels]])
            w.writerows(rows)
        print(f"\nCSV exportado para: {kpaper / 'summary.csv'} (e <M>/analysis.csv)")


if __name__ == "__main__":
    main()
