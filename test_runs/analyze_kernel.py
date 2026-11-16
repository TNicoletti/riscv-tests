#!/usr/bin/env python3
"""
Analisador de UM teste kernel: quais falhas M1-M6 ele detecta, como (crash / check / diff)
e em qual seção do teste. Lê as saídas do multi_run_kernels.py; critérios em kernel_analysis.py.

    python3 test_runs/analyze_kernel.py KERNEL [KPAPER_DIR] [-e M1,M6] [--list]

KERNEL: mask_agnostic, tail_agnostic, vector_compare, vlen, lmul, data_hazards, interruption
        (com ou sem o prefixo test_)
--list  mostra todas as falhas detectadas (e as só diferentes do golden)
"""

import argparse
import sys
from collections import Counter

from kernel_analysis import (DEFAULT_KPAPER_DIR, EXPERIMENTS, KERNELS, analyze_experiment,
                             groups, load_golden, pct, resolve_dir)


def main():
    parser = argparse.ArgumentParser(description="Resumo das falhas M1-M6 em um teste kernel.")
    parser.add_argument("kernel", help=", ".join(KERNELS))
    parser.add_argument("kpaper_dir", nargs="?", default=str(DEFAULT_KPAPER_DIR),
                        help=f"pasta de resultados (padrão: {DEFAULT_KPAPER_DIR})")
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS))
    parser.add_argument("--list", action="store_true", help="lista as falhas detectadas")
    args = parser.parse_args()

    kernel = args.kernel.removeprefix("test_")
    if kernel not in KERNELS:
        sys.exit(f"Kernel desconhecido: {args.kernel} (opções: {', '.join(KERNELS)})")
    kpaper = resolve_dir(args.kpaper_dir)
    if not kpaper.is_dir():
        sys.exit(f"Erro: '{kpaper}' não encontrado.")
    golden = load_golden(kpaper)
    experiments = [e.strip().upper() for e in args.experiments.split(",") if e.strip()]

    print("=" * 86)
    print(f" test_{kernel}")
    print("=" * 86)
    if kernel in golden:
        fp = [r for r in analyze_experiment(kpaper, "golden", golden, [kernel]) if r.detected]
        print(f"Spike padrão (golden): {'FALSO POSITIVO (' + fp[0].status + ')' if fp else 'ok'}\n")

    print(f"{'modelo':<27} {'N':>4} {'detectou':>15} {'crash':>6} {'check':>6} {'só diff':>8} "
          f"{'tempo médio':>12}")
    details = []
    total = Counter()
    for experiment in experiments:
        runs = analyze_experiment(kpaper, experiment, golden, [kernel])
        for title, group in groups(experiment, runs, lambda r: r.fault):
            if not group:
                continue
            n = len(group)
            c = Counter(r.status for r in group)
            d = c["crash"] + c["check"]
            secs = [r.seconds for r in group if r.seconds is not None]
            mean = f"{sum(secs) / len(secs):.2f}s" if secs else "-"
            print(f"{experiment + ' ' + title:<27} {n:>4} {pct(d, n):>15} {c['crash']:>6} "
                  f"{c['check']:>6} {c['diff']:>8} {mean:>12}")
            total.update(c)
            total["n"] += n
            details.append((experiment, title, group))
    if not details:
        sys.exit(f"Nenhum resultado de test_{kernel} em '{kpaper}'.")
    d = total["crash"] + total["check"]
    print(f"{'Total':<27} {total['n']:>4} {pct(d, total['n']):>15} {total['crash']:>6} "
          f"{total['check']:>6} {total['diff']:>8}")
    print("  crash = travou/rc != 0; check = o kernel acusou erro; só diff = saída diferente do "
          "golden sem acusar")

    # Onde o kernel detecta
    where = Counter(r.where or "(sem seção)" for _, _, g in details for r in g if r.detected)
    if where:
        print("\nSeções em que detectou:")
        for w, n in where.most_common():
            print(f"  {n:>4}  {w}")

    if args.list:
        for experiment, title, group in details:
            hits = [r for r in group if r.status != "ok"]
            if not hits:
                continue
            print(f"\n{experiment} {title}:")
            for r in hits:
                extra = f"rc={r.rc}" if r.status == "crash" else ""
                print(f"  {r.fault:<45} {r.status:<6} {extra:<8} {r.where}")


if __name__ == "__main__":
    main()
