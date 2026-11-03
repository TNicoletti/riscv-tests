#!/usr/bin/env python3
"""
Adaptação do multi_run.py para os experimentos da Tabela II do artigo (M1-M6).

Roda cada falha com as mesmas seeds de entrada (ver abaixo), em vez de rodar
todos os arquivos de test_runs/DIST/inputsBITS.

    M1 Illegal instruction  --illegal-instruction I, para cada uma das 97 instruções cobertas
    M2 Wrong result         --faulty-instruction I, para cada uma das 97 instruções cobertas
                            (o Spike não tem o bit flip (e, b) do artigo; usa a troca de vs2)
    M3 RAW hazard           --RAW-register R, R = 0..31
    M4 Intermittent         --ndb-chance p --ndb-fault 1, p = 0.1, 0.01, 0.001, 10 repetições
                            (o ndb não recebe instrução: afeta qualquer instrução vetorial)
    M5 Sequence             20 triplas (--three-instruction-err) e 20 quádruplas
                            (--four-instruction-err) diferentes, sorteadas entre as instruções cobertas
    M6 Cold start           --cold-start=5,I para 10 instruções sorteadas

Todos os experimentos rodam as mesmas seeds. Uma seed tem 10^5 sequências: como
cada arquivo de entrada roda 1332 testes, uma seed são 76 arquivos int32
consecutivos de test_runs/DIST/inputsBITS (gerados pelo gen_files.py):

    seed 001: 001..076.int32.in   valores pequenos (-32 a 31)
    seed 002: 077..152.int32.in   valores maiores (-2^20 a 2^20-1)
    seed 003: 153..228.int32.in   faixa inteira do int32

Os arquivos de uma seed rodam em ordem, e a seed para no primeiro arquivo que
detecta a falha ("Going to error discoverer" ou "Failed on test" na saída);
os arquivos seguintes não rodam.
Seeds e falhas diferentes rodam em paralelo.

Também roda o Spike padrão (golden) com as mesmas seeds, para comparação.

Saídas:  ./benchmarks/BENCH/test_runs/paper/<M>/<falha>/<seed>/<entrada>.out
Resumo:  ./benchmarks/BENCH/test_runs/paper/<M>/manifest.csv   (uma linha por arquivo rodado)
         ./benchmarks/BENCH/test_runs/paper/<M>/seeds.csv      (uma linha por seed: tempo e
                                                                sequências até detectar)

No final imprime, por experimento e seed, o tempo médio de uma seed e a média de
sequências executadas até detectar a falha.

Cada pasta <seed> pode ser analisada com detect_faults.py.

Uso:
    python3 test_runs/multi_run_paper.py BENCH [BITS] [-e M1,M3] [-n 3] [--dist uniform]
"""

import argparse
import csv
import os
import random
import re
import shutil
import signal
import subprocess
import sys
import threading
import time
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed

ISA = "rv64gcv_zvl128b_zicntr_zba_zbb"
EXPERIMENTS = ["M1", "M2", "M3", "M4", "M5", "M6"]

# 97 instruções vetoriais executadas (coverage.csv, sem vle32.v, vse32.v e vsetvli).
# Valores MATCH_* do encoding.h do Spike; o Spike compara só funct6|funct3|opcode
# (máscara 0xFC00707F), então os registradores não importam.
# Obs.: vcpop.m e vfirst.m têm os mesmos campos, então uma falha em uma afeta a outra.
COVERED_INSTRUCTIONS = {
    "vadd_vi": 0x3057,
    "vadd_vv": 0x57,
    "vadd_vx": 0x4057,
    "vand_vi": 0x24003057,
    "vand_vv": 0x24000057,
    "vand_vx": 0x24004057,
    "vcompress_vm": 0x5e002057,
    "vcpop_m": 0x40082057,
    "vdiv_vv": 0x84002057,
    "vdiv_vx": 0x84006057,
    "vdivu_vv": 0x80002057,
    "vdivu_vx": 0x80006057,
    "vmand_mm": 0x66002057,
    "vmax_vv": 0x1c000057,
    "vmax_vx": 0x1c004057,
    "vmaxu_vv": 0x18000057,
    "vmaxu_vx": 0x18004057,
    "vfirst_m": 0x4008a057,
    "vmin_vv": 0x14000057,
    "vmin_vx": 0x14004057,
    "vminu_vv": 0x10000057,
    "vminu_vx": 0x10004057,
    "vmnand_mm": 0x76002057,
    "vmor_mm": 0x6a002057,
    "vmseq_vi": 0x60003057,
    "vmseq_vv": 0x60000057,
    "vmseq_vx": 0x60004057,
    "vmsgt_vi": 0x7c003057,
    "vmsgt_vx": 0x7c004057,
    "vmsle_vi": 0x74003057,
    "vmsle_vv": 0x74000057,
    "vmsle_vx": 0x74004057,
    "vmsleu_vv": 0x70000057,
    "vmsleu_vx": 0x70004057,
    "vmslt_vv": 0x6c000057,
    "vmslt_vx": 0x6c004057,
    "vmsltu_vv": 0x68000057,
    "vmsltu_vx": 0x68004057,
    "vmsne_vi": 0x64003057,
    "vmsne_vv": 0x64000057,
    "vmsne_vx": 0x64004057,
    "vmul_vv": 0x94002057,
    "vmul_vx": 0x94006057,
    "vmulh_vv": 0x9c002057,
    "vmulhsu_vv": 0x98002057,
    "vmulhsu_vx": 0x98006057,
    "vmulhu_vv": 0x90002057,
    "vmulhu_vx": 0x90006057,
    "vmv_v_i": 0x5e003057,
    "vmv_v_v": 0x5e000057,
    "vmv_v_x": 0x5e004057,
    "vmxor_mm": 0x6e002057,
    "vor_vi": 0x28003057,
    "vor_vv": 0x28000057,
    "vor_vx": 0x28004057,
    "vredand_vs": 0x4002057,
    "vredmax_vs": 0x1c002057,
    "vredmaxu_vs": 0x18002057,
    "vredmin_vs": 0x14002057,
    "vredminu_vs": 0x10002057,
    "vredor_vs": 0x8002057,
    "vredsum_vs": 0x2057,
    "vredxor_vs": 0xc002057,
    "vrem_vv": 0x8c002057,
    "vrem_vx": 0x8c006057,
    "vremu_vv": 0x88002057,
    "vremu_vx": 0x88006057,
    "vslidedown_vi": 0x3c003057,
    "vslideup_vi": 0x38003057,
    "vsll_vi": 0x94003057,
    "vsll_vv": 0x94000057,
    "vsll_vx": 0x94004057,
    "vsra_vi": 0xa4003057,
    "vsra_vv": 0xa4000057,
    "vsra_vx": 0xa4004057,
    "vsrl_vi": 0xa0003057,
    "vsrl_vv": 0xa0000057,
    "vsrl_vx": 0xa0004057,
    "vsub_vv": 0x8000057,
    "vsub_vx": 0x8004057,
    "vxor_vi": 0x2c003057,
    "vxor_vv": 0x2c000057,
    "vxor_vx": 0x2c004057,
    "vfadd_vf": 0x5057,
    "vfadd_vv": 0x1057,
    "vfdiv_vf": 0x80005057,
    "vfdiv_vv": 0x80001057,
    "vfmacc_vv": 0xb0001057,
    "vfmul_vf": 0x90005057,
    "vfmul_vv": 0x90001057,
    "vfsqrt_v": 0x4c001057,
    "vfsub_vf": 0x8005057,
    "vfsub_vv": 0x8001057,
    "vmfeq_vv": 0x60001057,
    "vmfle_vv": 0x64001057,
    "vmflt_vv": 0x6c001057,
    "vmfne_vv": 0x70001057,
}

NDB_CHANCES = ["0.1", "0.01", "0.001"]
NDB_REPETITIONS = 10
SEQUENCES_PER_SIZE = 20
SEQUENCES_PER_SEED = 10**5
TESTS_PER_FILE = 1332
COLD_START_INSTRUCTIONS = 10
COLD_START_N = 5

# Mensagens do programa quando detecta a falha. "Going to error discoverer" sai logo
# na divergência; "Failed on test" só depois do diagnóstico, que às vezes não termina.
DETECTED_PATTERN = re.compile(r"Going to error discoverer|==== Failed on test \d+ ====")
# "Divergence z-(z+inc)": a falha apareceu no teste z / inc do arquivo
DIVERGENCE_PATTERN = re.compile(r"Divergence (\d+)-(\d+)")

# Processos spike atualmente em execução
running_processes = set()
process_lock = threading.Lock()
stopping = threading.Event()


def instr_hex(name):
    return f"0x{COVERED_INSTRUCTIONS[name]:08x}"


def build_faults(experiments, seed):
    """Lista de (experimento, id da falha, argumentos do spike)."""
    rng = random.Random(seed)
    names = list(COVERED_INSTRUCTIONS)
    faults = []

    if "M1" in experiments:
        for name in names:
            faults.append(("M1", name, ["--illegal-instruction", instr_hex(name)]))

    if "M2" in experiments:
        for name in names:
            faults.append(("M2", name, ["--faulty-instruction", instr_hex(name)]))

    if "M3" in experiments:
        for reg in range(32):
            faults.append(("M3", f"v{reg}", ["--RAW-register", f"0x{reg:02x}"]))

    if "M4" in experiments:
        for chance in NDB_CHANCES:
            for rep in range(NDB_REPETITIONS):
                faults.append((
                    "M4",
                    f"p{chance}_r{rep}",
                    ["--ndb-chance", chance, "--ndb-fault", "1"],
                ))

    if "M5" in experiments:
        for size, flag in ((3, "three"), (4, "four")):
            sequences = []
            while len(sequences) < SEQUENCES_PER_SIZE:
                seq = tuple(rng.choice(names) for _ in range(size))
                if seq not in sequences:
                    sequences.append(seq)
            for i, seq in enumerate(sequences):
                faults.append((
                    "M5",
                    f"{flag}{i:02d}_" + "-".join(seq),
                    [f"--{flag}-instruction-err=" + ",".join(instr_hex(n) for n in seq)],
                ))

    if "M6" in experiments:
        for name in rng.sample(names, COLD_START_INSTRUCTIONS):
            faults.append(("M6", name, [f"--cold-start={COLD_START_N},{instr_hex(name)}"]))

    return faults


def prepare_binaries(bench, input_files, tmp_dir):
    """Gera um .riscv por entrada (com .OUT_SECTION atualizada), reutilizado por todas as falhas."""
    base_riscv = Path(f"./benchmarks/{bench}.riscv")
    binaries = {}

    for input_file in input_files:
        temp_riscv = tmp_dir / f"{input_file.stem}.riscv"
        shutil.copy2(base_riscv, temp_riscv)
        subprocess.run(
            [
                "riscv64-unknown-elf-objcopy",
                "--update-section",
                f".OUT_SECTION={input_file}",
                str(temp_riscv),
            ],
            check=True,
        )
        binaries[input_file] = temp_riscv

    return binaries


def run_test(spike, fault_args, riscv, output_file, timeout):
    """Roda o spike e retorna (código de retorno ou 'timeout', segundos)."""
    if stopping.is_set():
        return "cancelado", 0.0

    cmd = [spike, f"--isa={ISA}", *fault_args, str(riscv)]
    start = time.monotonic()

    with open(output_file, "w") as f:
        # stderr junto do stdout: <SPIKE_DMG>, ALO e mensagens de trap ficam na saída
        process = subprocess.Popen(cmd, stdout=f, stderr=subprocess.STDOUT)

        with process_lock:
            running_processes.add(process)

        try:
            rc = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
            rc = "timeout"
        finally:
            with process_lock:
                running_processes.discard(process)

    return rc, time.monotonic() - start


def sequences_until(output, files_before):
    """Sequências executadas até a falha: arquivos anteriores inteiros + testes deste."""
    m = DIVERGENCE_PATTERN.search(output)
    if not m:
        return None
    start, end = int(m.group(1)), int(m.group(2))
    return files_before * TESTS_PER_FILE + start // (end - start) + 1


def run_seed(spike, fault_args, files, binaries, seed_dir, timeout):
    """Roda os arquivos da seed em ordem até um detectar a falha.

    Retorna (linhas do manifest, arquivo que detectou ou None,
             sequências até detectar ou None, segundos da seed)."""
    rows = []
    start = time.monotonic()
    for i, input_file in enumerate(files):
        if stopping.is_set():
            break
        output_file = seed_dir / (input_file.name.removesuffix(".in") + ".out")
        rc, seconds = run_test(spike, fault_args, binaries[input_file], output_file, timeout)
        if rc == "cancelado":
            break
        rows.append([input_file.name, rc, f"{seconds:.2f}", str(output_file)])
        output = output_file.read_text(encoding="utf-8", errors="ignore")
        if DETECTED_PATTERN.search(output):
            return rows, input_file.name, sequences_until(output, i), time.monotonic() - start
    return rows, None, None, time.monotonic() - start


def stop_running_processes():
    """Encerra todos os Spikes atualmente em execução."""
    stopping.set()

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

    for process in processes:
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            try:
                process.kill()
            except ProcessLookupError:
                pass


def main():
    parser = argparse.ArgumentParser(
        description="Roda os experimentos M1-M6 do artigo com o Spike modificado.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("bench")
    parser.add_argument("bits", nargs="?", default="32")
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS),
                        help="lista separada por vírgula (padrão: todos)")
    parser.add_argument("-n", "--seeds", type=int, default=3,
                        help="número de seeds por falha (padrão: 3)")
    parser.add_argument("--dist", default="uniform",
                        help="distribuição de onde vêm as entradas (padrão: uniform)")
    parser.add_argument("--seed", type=int, default=0,
                        help="semente para sortear as sequências (M5) e as instruções (M6)")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 1)
    parser.add_argument("--timeout", type=float, default=60, help="segundos por execução")
    parser.add_argument("--spike", default="spike", help="binário do spike (padrão: spike)")
    parser.add_argument("--no-golden", action="store_true",
                        help="não roda o spike padrão")
    parser.add_argument("-o", "--out-dir",
                        help="pasta dos resultados (padrão: benchmarks/BENCH/test_runs/paper)")
    args = parser.parse_args()

    if os.path.basename(os.getcwd()) == "test_runs":
        os.chdir("..")

    experiments = [e.strip().upper() for e in args.experiments.split(",") if e.strip()]
    unknown = set(experiments) - set(EXPERIMENTS)
    if unknown:
        sys.exit(f"Experimento(s) desconhecido(s): {', '.join(sorted(unknown))}")

    bench = args.bench
    if bench == "random_permutation_test":  # TODO: REMOVE
        bench = "random_neural_network"  # TODO: REMOVE

    # Resolve antes do chdir para que -o seja relativo ao diretório de chamada
    out_dir = Path(args.out_dir).resolve() if args.out_dir else None

    # Seeds: blocos consecutivos de arquivos int32, sem repetir arquivo entre as seeds
    input_dir = Path(f"./test_runs/{args.dist}/inputs{args.bits}")
    all_inputs = sorted(input_dir.glob(f"*.int{args.bits}.in"))
    files_per_seed = -(-SEQUENCES_PER_SEED // TESTS_PER_FILE)
    if len(all_inputs) < args.seeds * files_per_seed:
        sys.exit(f"{input_dir}: {args.seeds} seeds precisam de {args.seeds * files_per_seed} "
                 f"arquivos int{args.bits}, há {len(all_inputs)}")
    seeds = [
        (f"{k + 1:03d}", all_inputs[k * files_per_seed:(k + 1) * files_per_seed])
        for k in range(args.seeds)
    ]

    faults = build_faults(experiments, args.seed)
    if not args.no_golden:
        faults.insert(0, ("golden", "golden", []))

    out_root = out_dir or Path(f"./benchmarks/{bench}/test_runs/paper")
    for experiment in {f[0] for f in faults}:
        exp_dir = out_root / experiment
        if exp_dir.exists():
            shutil.rmtree(exp_dir)

    subprocess.run(
        ["python3", "./mem_setter.py", f"./benchmarks/{bench}/params.json"],
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

    tmp_dir = out_root / "tmp"
    tmp_dir.mkdir(parents=True, exist_ok=True)

    for name, files in seeds:
        print(f"Seed {name}: {files[0].name} .. {files[-1].name} ({len(files)} arquivos)")
    counts = {}
    for experiment, *_ in faults:
        counts[experiment] = counts.get(experiment, 0) + 1
    print("Falhas: " + ", ".join(f"{e}={n}" for e, n in counts.items()))
    print(f"Total: {len(faults) * len(seeds)} seeds, até "
          f"{len(faults) * sum(len(files) for _, files in seeds)} execuções\n")

    rows = []
    seed_rows = []
    futures = {}

    try:
        binaries = prepare_binaries(bench, [f for _, files in seeds for f in files], tmp_dir)

        with ThreadPoolExecutor(max_workers=args.jobs) as executor:
            for experiment, fault_id, fault_args in faults:
                for seed_name, files in seeds:
                    seed_dir = out_root / experiment / fault_id / seed_name
                    seed_dir.mkdir(parents=True, exist_ok=True)
                    future = executor.submit(
                        run_seed, args.spike, fault_args, files, binaries, seed_dir, args.timeout
                    )
                    futures[future] = (experiment, fault_id, " ".join(fault_args), seed_name)

            done = 0
            for future in as_completed(futures):
                experiment, fault_id, spike_args, seed_name = futures[future]
                file_rows, detected_by, sequences, seed_seconds = future.result()
                done += 1

                for input_name, rc, seconds, output_file in file_rows:
                    rows.append([experiment, fault_id, spike_args, seed_name, input_name, rc,
                                 seconds, output_file])
                    if rc != 0:
                        print(f"[{done}/{len(futures)}] {experiment} {fault_id} {seed_name} "
                              f"{input_name}: rc={rc}")

                seed_rows.append([
                    experiment, fault_id, spike_args, seed_name,
                    "sim" if detected_by else "nao", len(file_rows), detected_by or "",
                    "" if sequences is None else sequences, f"{seed_seconds:.2f}",
                ])
                if done % 50 == 0:
                    print(f"[{done}/{len(futures)} seeds]")

    except KeyboardInterrupt:
        print("\nCtrl+C recebido!")
        for future in futures:
            future.cancel()
        stop_running_processes()
        sys.exit(130)

    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)

        # Um manifest por experimento, para não perder o de rodadas anteriores
        for experiment in sorted({r[0] for r in seed_rows}):
            manifest = out_root / experiment / "manifest.csv"
            with open(manifest, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["experiment", "fault", "spike_args", "seed", "input",
                            "return_code", "seconds", "output"])
                w.writerows(sorted((r for r in rows if r[0] == experiment),
                                   key=lambda r: (r[1], r[3], r[4])))

            seeds_csv = out_root / experiment / "seeds.csv"
            with open(seeds_csv, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["experiment", "fault", "spike_args", "seed", "detected",
                            "files_run", "detected_by", "sequences", "seconds"])
                w.writerows(sorted((r for r in seed_rows if r[0] == experiment),
                                   key=lambda r: (r[1], r[3])))
            print(f"Resumo em {manifest} e {seeds_csv}")

        print_timing(seed_rows)


def print_timing(seed_rows):
    """Tempo por seed e sequências até detectar, por experimento e seed."""
    if not seed_rows:
        return
    print(f"\n{'exp.':<7} {'seed':<5} {'seeds':>5} {'detect.':>8} {'tempo médio':>12} "
          f"{'tempo total':>12} {'seq. média':>11} {'seq. mediana':>13}")
    groups = {}
    for r in seed_rows:
        groups.setdefault((r[0], r[3]), []).append(r)
    order = {e: i for i, e in enumerate(["golden"] + EXPERIMENTS)}
    for (experiment, seed_name), group in sorted(groups.items(), key=lambda g: (order[g[0][0]], g[0][1])):
        seconds = [float(r[8]) for r in group]
        sequences = sorted(int(r[7]) for r in group if r[7] != "")
        mean_seq = f"{sum(sequences) / len(sequences):.0f}" if sequences else "-"
        median_seq = str(sequences[len(sequences) // 2]) if sequences else "-"
        print(f"{experiment:<7} {seed_name:<5} {len(group):>5} {len(sequences):>8} "
              f"{sum(seconds) / len(seconds):>11.2f}s {sum(seconds):>11.1f}s "
              f"{mean_seq:>11} {median_seq:>13}")


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, lambda *_: (stop_running_processes(), sys.exit(143)))
    main()
