#!/usr/bin/env python3
import argparse
import os
import subprocess
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

# --- CONFIGURAÇÃO DECLARATIVA DOS CASOS DE TESTE ---
UNIFORM_RUNS = [
    {"count": 100, "low": -32,      "high": 32},
    {"count": 200, "low": -64,      "high": 64},
    {"count": 200, "low": -256,     "high": 256},
    {"count": 400, "low": -32768,   "high": 32768},
    {"count": 1300,"low": -1048576, "high": 1048576},
]

DATA_TYPES = ["float32", "int32"]

def setup_directory():
    cwd = Path.cwd()
    if cwd.name != "test_runs":
        target = cwd / "test_runs"
        if target.is_dir():
            os.chdir(target)

def clean_inputs():
    categories = ["normal", "log_normal", "uniform", "all_ones", "all_zeros"]
    for cat in categories:
        inputs_dir = Path(cat) / "inputs"
        if inputs_dir.exists():
            for file in inputs_dir.glob("*"):
                if file.is_file():
                    file.unlink()

def run_single_job(job_args):
    """
    Função executada pelos workers em paralelo.
    job_args é uma tupla: (outfile, args_list, regenerate)
    """
    outfile, args_list, regenerate = job_args
    outfile.parent.mkdir(parents=True, exist_ok=True)

    if not regenerate and outfile.exists():
        return

    cmd = ["python3", "rng.py"] + args_list + ["-o", str(outfile)]
    subprocess.run(cmd, check=True)

def main():
    parser = argparse.ArgumentParser(description="Gerador automático de testes paralelizado.")
    parser.add_argument("-r", "--regenerate", action="store_true", help="Força a regeneração.")
    parser.add_argument("--start-seed", type=int, default=1, help="Seed inicial (padrão: 1).")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count(), 
                        help=f"Número de processos paralelos (padrão: {os.cpu_count()}).")
    args = parser.parse_args()

    setup_directory()
    clean_inputs()

    base_folder = Path.cwd()
    n_elements = 32768
    current_seed = args.start_seed

    # 1. Monta a fila de tarefas (jobs)
    tasks = []
    for run in UNIFORM_RUNS:
        count = run["count"]
        low, high = run["low"], run["high"]

        for _ in range(count):
            for dtype in DATA_TYPES:
                outfile = base_folder / "uniform" / "inputs" / f"{current_seed}.{dtype}.in"
                cmd_args = [
                    "-n", str(n_elements),
                    "-s", str(current_seed),
                    "-d", "uniform",
                    "-t", dtype,
                    "--low", str(low),
                    "--high", str(high)
                ]
                tasks.append((outfile, cmd_args, args.regenerate))
            
            current_seed += 1

    total_tasks = len(tasks)
    print(f"Gerando {total_tasks} arquivos usando {args.jobs} processos em paralelo...")

    # 2. Executa as tarefas em paralelo
    completed = 0
    with ProcessPoolExecutor(max_workers=args.jobs) as executor:
        futures = [executor.submit(run_single_job, task) for task in tasks]
        
        for _ in as_completed(futures):
            completed += 1
            if completed % 100 == 0 or completed == total_tasks:
                print(f"Progresso: {completed}/{total_tasks} arquivos processados...", end="\r")

    print(f"\nPronto! Concluído em paralelo. Seeds de {args.start_seed} até {current_seed - 1}.")

if __name__ == "__main__":
    main()