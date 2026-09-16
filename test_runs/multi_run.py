#!/usr/bin/env python3

import os
import sys
import shutil
import signal
import subprocess
import threading
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed


# Processos spike atualmente em execução
running_processes = set()
process_lock = threading.Lock()


def run_test(input_file, output_file, base_riscv, temp_riscv):
    try:
        # Copia o .riscv original para um arquivo exclusivo desta execução
        shutil.copy2(base_riscv, temp_riscv)

        # Atualiza a seção .OUT_SECTION
        subprocess.run(
            [
                "riscv64-unknown-elf-objcopy",
                "--update-section",
                f".OUT_SECTION={input_file}",
                str(temp_riscv),
            ],
            check=True,
        )

        # Executa o Spike
        with open(output_file, "w") as f:
            process = subprocess.Popen(
                [
                    "spike",
                    "--isa=rv64gcv_zvl128b_zicntr_zba_zbb",
                    str(temp_riscv),
                ],
                stdout=f,
            )

            # Registra o processo
            with process_lock:
                running_processes.add(process)

            try:
                return_code = process.wait()

                if return_code != 0:
                    raise subprocess.CalledProcessError(
                        return_code,
                        process.args,
                    )

            finally:
                with process_lock:
                    running_processes.discard(process)

        print(f"[OK] {input_file}")

    except subprocess.CalledProcessError as e:
        print(f"[ERRO] {input_file}: comando retornou {e.returncode}")

    except Exception as e:
        print(f"[ERRO] {input_file}: {e}")

    finally:
        # Remove o .riscv temporário mesmo se ocorrer erro
        if temp_riscv.exists():
            try:
                temp_riscv.unlink()
            except FileNotFoundError:
                pass


def stop_running_processes():
    """Encerra todos os Spikes atualmente em execução."""

    with process_lock:
        processes = list(running_processes)

    if not processes:
        return

    print(f"\nInterrompendo {len(processes)} processo(s) Spike...")

    for process in processes:
        if process.poll() is None:
            try:
                process.terminate()
            except ProcessLookupError:
                pass

    # Dá um pequeno tempo para terminarem normalmente
    for process in processes:
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            try:
                process.kill()
            except ProcessLookupError:
                pass


def run_distribution(folder, bench):
    input_dir = Path(f"./test_runs/{folder}/inputs")
    output_dir = Path(f"./benchmarks/{bench}/test_runs/{folder}/outputs")
    base_riscv = Path(f"./benchmarks/{bench}.riscv")

    print(f"\n=== {folder} ===")

    if not input_dir.is_dir():
        print(f"Aviso: Diretório {input_dir} não encontrado. Pulando...")
        return

    output_dir.mkdir(parents=True, exist_ok=True)

    input_files = sorted(input_dir.glob("*.in"))

    if not input_files:
        print(f"Nenhum arquivo .in encontrado em {input_dir}")
        return

    futures = []

    max_workers = os.cpu_count() or 1

    try:
        with ThreadPoolExecutor(max_workers=max_workers) as executor:

            for input_file in input_files:
                filename = input_file.name
                output_file = output_dir / (
                    filename.removesuffix(".in") + ".out"
                )

                temp_riscv = input_dir / f"{input_file.stem}.riscv"

                futures.append(
                    executor.submit(
                        run_test,
                        input_file,
                        output_file,
                        base_riscv,
                        temp_riscv,
                    )
                )

            for future in as_completed(futures):
                future.result()

    except KeyboardInterrupt:
        print("\nCtrl+C recebido!")

        # Cancela tarefas que ainda não começaram
        for future in futures:
            future.cancel()

        # Mata os Spikes que já começaram
        stop_running_processes()

        # Não deixa o executor esperar pelas threads
        raise

def remove_previous(bench):
    for folder in ["uniform", "normal", "log_normal", "all_ones", "all_zeros"]:
        output_dir = Path(f"./benchmarks/{bench}/test_runs/{folder}/outputs")
        if output_dir.exists():
            for output_file in output_dir.glob("*.out"):
                output_file.unlink()

def main():
    if len(sys.argv) != 2:
        print(f"Uso: {sys.argv[0]} BENCH")
        sys.exit(1)

    if os.path.basename(os.getcwd()) == "test_runs":
        os.chdir("..")

    bench = sys.argv[1]


    if bench == "random_permutation_test":  # TODO: REMOVE
        bench = "random_neural_network"  # TODO: REMOVE

    remove_previous(bench)

    subprocess.run(
        [
            "python3",
            "./mem_setter.py",
            f"./benchmarks/{bench}/params.json"
        ],
        check=True,
    )
    
    subprocess.run(
        [
            "riscv64-unknown-elf-objcopy",
            "--update-section",
            f".PARAMETERS_SECTION=./benchmarks/{bench}/params.mem",
            f"./benchmarks/{bench}.riscv",
        ],
        check=True,
    )

    try:
        for folder in [
            "uniform",
            "normal",
            "log_normal",
            "all_ones",
            "all_zeros",
        ]:
            run_distribution(folder, bench)

    except KeyboardInterrupt:
        print("\nExecução interrompida pelo usuário.")
        stop_running_processes()
        sys.exit(130)


if __name__ == "__main__":
    main()