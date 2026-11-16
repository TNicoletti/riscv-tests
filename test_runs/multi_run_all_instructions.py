#!/usr/bin/env python3
"""
Roda as falhas M1-M6 do artigo (Tabela II) no benchmark test_all_instructions.

Diferente do random_neural_network (multi_run_paper), aqui não há seeds: cada Spike
defeituoso roda UMA vez. O test_all_instructions executa cada instrução sozinha
`repeat_instruction` vezes (params.json) e imprime, por instrução, o resultado de cada
repetição ([XX] certo, [WA] resposta errada, [II] instrução ilegal, ...) e PASS/ERR.

As falhas são as mesmas do multi_run_paper (mesma lista de 97 instruções e, com o mesmo
--seed, as mesmas sequências do M5 e instruções do M6):

    M1 Illegal instruction  --illegal-instruction I, para cada uma das 97 instruções
    M2 Wrong result         --faulty-instruction I, para cada uma das 97 instruções
    M3 RAW hazard           --RAW-register R, R = 0..31
    M4 Intermittent         --ndb-chance p --ndb-fault 1, p = 0.1, 0.01, 0.001, 10 repetições
    M5 Sequence             20 triplas e 20 quádruplas sorteadas
    M6 Cold start           --cold-start=5,I para 10 instruções sorteadas

Valores iniciais: o programa não gera os dados (generate_initial_values está comentado), lê
da .OUT_SECTION. Ela recebe um arquivo de entrada aleatório de test_runs/DIST/inputsBITS
(padrão: 153.int32.in, faixa inteira do int32; troque com --input).

Parâmetros: o params.json do benchmark (ou --params) é convertido com mem_setter.py e
gravado na .PARAMETERS_SECTION de uma cópia do binário (o .riscv original não muda).

Saídas:  ./benchmarks/test_all_instructions/test_runs/paper/<M>/<falha>.out
         ./benchmarks/test_all_instructions/test_runs/paper/<M>/manifest.csv
         ./benchmarks/test_all_instructions/test_runs/paper/golden/golden.out  (Spike padrão)

Análise: python3 test_runs/analyze_all_instructions.py

Uso:
    python3 test_runs/multi_run_all_instructions.py [-e M1,M3] [--input ARQ] [--params JSON]
"""

import argparse
import csv
import json
import os
import random
import shutil
import signal
import subprocess
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

BENCH = "test_all_instructions"
ISA = "rv64gcv_zvl128b_zicntr_zba_zbb"
EXPERIMENTS = ["M1", "M2", "M3", "M4", "M5", "M6"]

# Mesmas 97 instruções do multi_run_paper (coverage.csv do random_neural_network).
# Valores MATCH_* do encoding.h do Spike; o Spike compara só funct6|funct3|opcode
# (máscara 0xFC00707F). vcpop.m e vfirst.m têm os mesmos campos.
COVERED_INSTRUCTIONS = {
    "vadd_vi": 0x3057, "vadd_vv": 0x57, "vadd_vx": 0x4057,
    "vand_vi": 0x24003057, "vand_vv": 0x24000057, "vand_vx": 0x24004057,
    "vcompress_vm": 0x5e002057, "vcpop_m": 0x40082057,
    "vdiv_vv": 0x84002057, "vdiv_vx": 0x84006057, "vdivu_vv": 0x80002057, "vdivu_vx": 0x80006057,
    "vmand_mm": 0x66002057, "vmax_vv": 0x1c000057, "vmax_vx": 0x1c004057,
    "vmaxu_vv": 0x18000057, "vmaxu_vx": 0x18004057, "vfirst_m": 0x4008a057,
    "vmin_vv": 0x14000057, "vmin_vx": 0x14004057, "vminu_vv": 0x10000057, "vminu_vx": 0x10004057,
    "vmnand_mm": 0x76002057, "vmor_mm": 0x6a002057,
    "vmseq_vi": 0x60003057, "vmseq_vv": 0x60000057, "vmseq_vx": 0x60004057,
    "vmsgt_vi": 0x7c003057, "vmsgt_vx": 0x7c004057,
    "vmsle_vi": 0x74003057, "vmsle_vv": 0x74000057, "vmsle_vx": 0x74004057,
    "vmsleu_vv": 0x70000057, "vmsleu_vx": 0x70004057,
    "vmslt_vv": 0x6c000057, "vmslt_vx": 0x6c004057, "vmsltu_vv": 0x68000057, "vmsltu_vx": 0x68004057,
    "vmsne_vi": 0x64003057, "vmsne_vv": 0x64000057, "vmsne_vx": 0x64004057,
    "vmul_vv": 0x94002057, "vmul_vx": 0x94006057, "vmulh_vv": 0x9c002057,
    "vmulhsu_vv": 0x98002057, "vmulhsu_vx": 0x98006057, "vmulhu_vv": 0x90002057, "vmulhu_vx": 0x90006057,
    "vmv_v_i": 0x5e003057, "vmv_v_v": 0x5e000057, "vmv_v_x": 0x5e004057, "vmxor_mm": 0x6e002057,
    "vor_vi": 0x28003057, "vor_vv": 0x28000057, "vor_vx": 0x28004057,
    "vredand_vs": 0x4002057, "vredmax_vs": 0x1c002057, "vredmaxu_vs": 0x18002057,
    "vredmin_vs": 0x14002057, "vredminu_vs": 0x10002057, "vredor_vs": 0x8002057,
    "vredsum_vs": 0x2057, "vredxor_vs": 0xc002057,
    "vrem_vv": 0x8c002057, "vrem_vx": 0x8c006057, "vremu_vv": 0x88002057, "vremu_vx": 0x88006057,
    "vslidedown_vi": 0x3c003057, "vslideup_vi": 0x38003057,
    "vsll_vi": 0x94003057, "vsll_vv": 0x94000057, "vsll_vx": 0x94004057,
    "vsra_vi": 0xa4003057, "vsra_vv": 0xa4000057, "vsra_vx": 0xa4004057,
    "vsrl_vi": 0xa0003057, "vsrl_vv": 0xa0000057, "vsrl_vx": 0xa0004057,
    "vsub_vv": 0x8000057, "vsub_vx": 0x8004057,
    "vxor_vi": 0x2c003057, "vxor_vv": 0x2c000057, "vxor_vx": 0x2c004057,
    "vfadd_vf": 0x5057, "vfadd_vv": 0x1057, "vfdiv_vf": 0x80005057, "vfdiv_vv": 0x80001057,
    "vfmacc_vv": 0xb0001057, "vfmul_vf": 0x90005057, "vfmul_vv": 0x90001057, "vfsqrt_v": 0x4c001057,
    "vfsub_vf": 0x8005057, "vfsub_vv": 0x8001057,
    "vmfeq_vv": 0x60001057, "vmfle_vv": 0x64001057, "vmflt_vv": 0x6c001057, "vmfne_vv": 0x70001057,
}

NDB_CHANCES = ["0.1", "0.01", "0.001"]
NDB_REPETITIONS = 10
SEQUENCES_PER_SIZE = 20
COLD_START_INSTRUCTIONS = 10
COLD_START_N = 5

# OUT usado pelo programa: EL_PER_BLOCK * repeat * SUPORTED_INSTRUCTIONS * NUM_REGISTERS int32
EL_PER_BLOCK = 4
SUPORTED_INSTRUCTIONS = 113
NUM_REGISTERS = 3

running_processes = set()
process_lock = threading.Lock()
stopping = threading.Event()


def instr_hex(name):
    return f"0x{COVERED_INSTRUCTIONS[name]:08x}"


def build_faults(experiments, seed):
    """Lista de (experimento, id da falha, argumentos do spike). Igual ao multi_run_paper."""
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
                faults.append(("M4", f"p{chance}_r{rep}",
                               ["--ndb-chance", chance, "--ndb-fault", "1"]))

    if "M5" in experiments:
        for size, flag in ((3, "three"), (4, "four")):
            sequences = []
            while len(sequences) < SEQUENCES_PER_SIZE:
                seq = tuple(rng.choice(names) for _ in range(size))
                if seq not in sequences:
                    sequences.append(seq)
            for i, seq in enumerate(sequences):
                faults.append(("M5", f"{flag}{i:02d}_" + "-".join(seq),
                               [f"--{flag}-instruction-err=" + ",".join(instr_hex(n) for n in seq)]))

    if "M6" in experiments:
        for name in rng.sample(names, COLD_START_INSTRUCTIONS):
            faults.append(("M6", name, [f"--cold-start={COLD_START_N},{instr_hex(name)}"]))

    return faults


def repeat_from_params(params):
    for var in json.loads(params.read_text()).get("variables", []):
        if var.get("name") == "repeat_instruction":
            return int(var["value"])
    return 6  # padrão do test_all_instructions.h


def prepare_binary(params, input_file, tmp_dir):
    """Cópia do .riscv com .PARAMETERS_SECTION (params.json) e .OUT_SECTION (entrada)."""
    subprocess.run(["python3", "./mem_setter.py", str(params)], check=True,
                   stdout=subprocess.DEVNULL)
    riscv = tmp_dir / f"{BENCH}.riscv"
    shutil.copy2(f"./benchmarks/{BENCH}.riscv", riscv)
    subprocess.run(["riscv64-unknown-elf-objcopy", "--update-section",
                    f".PARAMETERS_SECTION={params.parent / 'params.mem'}", str(riscv)], check=True)
    subprocess.run(["riscv64-unknown-elf-objcopy", "--update-section",
                    f".OUT_SECTION={input_file}", str(riscv)], check=True)
    return riscv


def run_test(spike, fault_args, riscv, output_file, timeout):
    """Roda o spike e retorna (código de retorno ou 'timeout', segundos)."""
    if stopping.is_set():
        return "cancelado", 0.0
    cmd = [spike, f"--isa={ISA}", *fault_args, str(riscv)]
    start = time.monotonic()
    with open(output_file, "w") as f:
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


def stop_running_processes():
    stopping.set()
    with process_lock:
        processes = list(running_processes)
    for process in processes:
        if process.poll() is None:
            try:
                process.kill()
            except ProcessLookupError:
                pass


def main():
    parser = argparse.ArgumentParser(
        description="Roda as falhas M1-M6 uma vez cada no test_all_instructions.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS),
                        help="lista separada por vírgula (padrão: todos)")
    parser.add_argument("--input", default="./test_runs/uniform/inputs32/0153.int32.in",
                        help="valores iniciais (.OUT_SECTION) (padrão: uniform/inputs32/0153.int32.in)")
    parser.add_argument("--params", default=f"./benchmarks/{BENCH}/params.json",
                        help="params.json usado (padrão: o do benchmark)")
    parser.add_argument("--seed", type=int, default=0,
                        help="semente das sequências (M5) e instruções (M6); a mesma do "
                             "multi_run_paper dá as mesmas falhas")
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 1)
    parser.add_argument("--timeout", type=float, default=60, help="segundos por execução")
    parser.add_argument("--spike", default="spike", help="binário do spike (padrão: spike)")
    parser.add_argument("--no-golden", action="store_true", help="não roda o spike padrão")
    parser.add_argument("-o", "--out-dir",
                        help=f"pasta dos resultados (padrão: benchmarks/{BENCH}/test_runs/paper)")
    args = parser.parse_args()

    # Caminhos passados pelo usuário são relativos ao diretório de chamada
    cwd = Path.cwd()
    input_file = (cwd / args.input).resolve() if args.input != parser.get_default("input") else None
    params = (cwd / args.params).resolve() if args.params != parser.get_default("params") else None
    out_dir = (cwd / args.out_dir).resolve() if args.out_dir else None
    if cwd.name == "test_runs":
        os.chdir("..")
    input_file = input_file or Path(args.input).resolve()
    params = params or Path(args.params).resolve()

    experiments = [e.strip().upper() for e in args.experiments.split(",") if e.strip()]
    unknown = set(experiments) - set(EXPERIMENTS)
    if unknown:
        sys.exit(f"Experimento(s) desconhecido(s): {', '.join(sorted(unknown))}")
    for path in (input_file, params, Path(f"./benchmarks/{BENCH}.riscv")):
        if not path.is_file():
            sys.exit(f"Erro: '{path}' não encontrado.")

    repeat = repeat_from_params(params)
    needed = EL_PER_BLOCK * repeat * SUPORTED_INSTRUCTIONS * NUM_REGISTERS * 4
    if input_file.stat().st_size < needed:
        sys.exit(f"Erro: {input_file} tem {input_file.stat().st_size} bytes; com "
                 f"repeat_instruction={repeat} o programa lê {needed}.")

    faults = build_faults(experiments, args.seed)
    if not args.no_golden:
        faults.insert(0, ("golden", "golden", []))

    out_root = out_dir or Path(f"./benchmarks/{BENCH}/test_runs/paper")
    for experiment in {f[0] for f in faults}:
        shutil.rmtree(out_root / experiment, ignore_errors=True)
        (out_root / experiment).mkdir(parents=True)
    tmp_dir = out_root / "tmp"
    tmp_dir.mkdir(parents=True, exist_ok=True)

    counts = {}
    for experiment, *_ in faults:
        counts[experiment] = counts.get(experiment, 0) + 1
    print(f"Entrada: {input_file}")
    print(f"Parâmetros: {params} (repeat_instruction={repeat})")
    print("Falhas: " + ", ".join(f"{e}={n}" for e, n in counts.items())
          + f"  ({len(faults)} execuções)\n")

    rows = []
    futures = {}
    try:
        riscv = prepare_binary(params, input_file, tmp_dir)
        with ThreadPoolExecutor(max_workers=args.jobs) as executor:
            for experiment, fault_id, fault_args in faults:
                output_file = out_root / experiment / f"{fault_id}.out"
                future = executor.submit(run_test, args.spike, fault_args, riscv, output_file,
                                         args.timeout)
                futures[future] = (experiment, fault_id, " ".join(fault_args), output_file)

            for done, future in enumerate(as_completed(futures), 1):
                experiment, fault_id, spike_args, output_file = futures[future]
                rc, seconds = future.result()
                if rc == "cancelado":
                    continue
                rows.append([experiment, fault_id, spike_args, input_file.name, rc,
                             f"{seconds:.2f}", str(output_file)])
                if rc != 0:
                    print(f"[{done}/{len(futures)}] {experiment} {fault_id}: rc={rc}")

    except KeyboardInterrupt:
        print("\nCtrl+C recebido!")
        for future in futures:
            future.cancel()
        stop_running_processes()
        sys.exit(130)

    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)
        for experiment in sorted({r[0] for r in rows}):
            manifest = out_root / experiment / "manifest.csv"
            with open(manifest, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["experiment", "fault", "spike_args", "input", "return_code",
                            "seconds", "output"])
                w.writerows(sorted((r for r in rows if r[0] == experiment), key=lambda r: r[1]))

    print(f"\n{'exp.':<7} {'falhas':>6} {'tempo médio':>12} {'tempo total':>12}")
    order = {e: i for i, e in enumerate(["golden"] + EXPERIMENTS)}
    for experiment in sorted({r[0] for r in rows}, key=order.get):
        secs = [float(r[5]) for r in rows if r[0] == experiment]
        print(f"{experiment:<7} {len(secs):>6} {sum(secs) / len(secs):>11.2f}s {sum(secs):>11.1f}s")
    print(f"\nResultados em {out_root}")
    print("Análise: python3 test_runs/analyze_all_instructions.py")


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, lambda *_: (stop_running_processes(), sys.exit(143)))
    main()
