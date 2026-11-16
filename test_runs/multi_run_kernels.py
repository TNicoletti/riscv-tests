#!/usr/bin/env python3
"""
Roda as falhas M1-M6 do artigo (Tabela II) nos testes kernels (S1 do artigo; os mesmos do
run_kernels.sh): cada Spike defeituoso roda uma vez cada kernel.

As falhas são as mesmas do multi_run_paper / multi_run_all_instructions (mesmas 97
instruções e, com o mesmo --seed, as mesmas sequências do M5 e instruções do M6).
Os kernels não leem entradas nem params.json.

Saídas:  ./test_runs/kernels/paper/<M>/<falha>/<kernel>.out
         ./test_runs/kernels/paper/<M>/manifest.csv
         ./test_runs/kernels/paper/golden/golden/<kernel>.out   (Spike padrão)

Análise: python3 test_runs/analyze_kernels.py        (todos os modelos)
         python3 test_runs/analyze_kernel.py KERNEL  (um kernel só)

Uso:
    python3 test_runs/multi_run_kernels.py [-e M1,M3] [-k lmul,vlen]
"""

import argparse
import csv
import os
import shutil
import signal
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

from multi_run_all_instructions import (EXPERIMENTS, build_faults, run_test,
                                        stop_running_processes)

KERNELS = [
    "mask_agnostic",
    "tail_agnostic",
    "vector_compare",
    "vlen",
    "lmul",
    "data_hazards",
    "interruption",
]
DEFAULT_OUT = Path("./test_runs/kernels/paper")


def main():
    parser = argparse.ArgumentParser(
        description="Roda as falhas M1-M6 uma vez em cada teste kernel.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS),
                        help="lista separada por vírgula (padrão: todos)")
    parser.add_argument("-k", "--kernels", default=",".join(KERNELS),
                        help="lista separada por vírgula (padrão: todos)")
    parser.add_argument("--seed", type=int, default=0,
                        help="semente das sequências (M5) e instruções (M6); a mesma do "
                             "multi_run_paper dá as mesmas falhas")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 1)
    parser.add_argument("--timeout", type=float, default=60, help="segundos por execução")
    parser.add_argument("--spike", default="spike", help="binário do spike (padrão: spike)")
    parser.add_argument("--no-golden", action="store_true", help="não roda o spike padrão")
    parser.add_argument("-o", "--out-dir", help=f"pasta dos resultados (padrão: {DEFAULT_OUT})")
    args = parser.parse_args()

    out_dir = Path(args.out_dir).resolve() if args.out_dir else None
    if Path.cwd().name == "test_runs":
        os.chdir("..")

    experiments = [e.strip().upper() for e in args.experiments.split(",") if e.strip()]
    unknown = set(experiments) - set(EXPERIMENTS)
    if unknown:
        sys.exit(f"Experimento(s) desconhecido(s): {', '.join(sorted(unknown))}")

    kernels = [k.strip().removeprefix("test_") for k in args.kernels.split(",") if k.strip()]
    for kernel in kernels:
        if not Path(f"./benchmarks/test_{kernel}.riscv").is_file():
            sys.exit(f"Erro: ./benchmarks/test_{kernel}.riscv não encontrado.")

    faults = build_faults(experiments, args.seed)
    if not args.no_golden:
        faults.insert(0, ("golden", "golden", []))

    out_root = out_dir or DEFAULT_OUT
    for experiment in {f[0] for f in faults}:
        shutil.rmtree(out_root / experiment, ignore_errors=True)

    counts = {}
    for experiment, *_ in faults:
        counts[experiment] = counts.get(experiment, 0) + 1
    print(f"Kernels: {', '.join(kernels)}")
    print("Falhas: " + ", ".join(f"{e}={n}" for e, n in counts.items())
          + f"  ({len(faults) * len(kernels)} execuções)\n")

    rows = []
    futures = {}
    try:
        with ThreadPoolExecutor(max_workers=args.jobs) as executor:
            for experiment, fault_id, fault_args in faults:
                fault_dir = out_root / experiment / fault_id
                fault_dir.mkdir(parents=True, exist_ok=True)
                for kernel in kernels:
                    output_file = fault_dir / f"{kernel}.out"
                    future = executor.submit(run_test, args.spike, fault_args,
                                             Path(f"./benchmarks/test_{kernel}.riscv"),
                                             output_file, args.timeout)
                    futures[future] = (experiment, fault_id, " ".join(fault_args), kernel,
                                       output_file)

            for future in as_completed(futures):
                experiment, fault_id, spike_args, kernel, output_file = futures[future]
                rc, seconds = future.result()
                if rc == "cancelado":
                    continue
                rows.append([experiment, fault_id, spike_args, kernel, rc, f"{seconds:.2f}",
                             str(output_file)])

    except KeyboardInterrupt:
        print("\nCtrl+C recebido!")
        for future in futures:
            future.cancel()
        stop_running_processes()
        sys.exit(130)

    finally:
        for experiment in sorted({r[0] for r in rows}):
            with open(out_root / experiment / "manifest.csv", "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["experiment", "fault", "spike_args", "kernel", "return_code",
                            "seconds", "output"])
                w.writerows(sorted((r for r in rows if r[0] == experiment),
                                   key=lambda r: (r[1], r[3])))

    print(f"{'exp.':<7} {'execuções':>9} {'rc != 0':>8} {'tempo médio':>12} {'tempo total':>12}")
    order = {e: i for i, e in enumerate(["golden"] + EXPERIMENTS)}
    for experiment in sorted({r[0] for r in rows}, key=order.get):
        group = [r for r in rows if r[0] == experiment]
        secs = [float(r[5]) for r in group]
        bad = sum(r[4] != 0 for r in group)
        print(f"{experiment:<7} {len(group):>9} {bad:>8} {sum(secs) / len(secs):>11.2f}s "
              f"{sum(secs):>11.1f}s")
    print(f"\nResultados em {out_root}")
    print("Análise: python3 test_runs/analyze_kernels.py")


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, lambda *_: (stop_running_processes(), sys.exit(143)))
    main()
