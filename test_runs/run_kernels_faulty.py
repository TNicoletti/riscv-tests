#!/usr/bin/env python3
"""
Roda os testes kernels (mesmos do run_kernels.sh) com as modificações do Spike.

Uso:
    python3 test_runs/run_kernels_faulty.py MODO [VALORES...] [opções]

Modos (ver test_runs/SPIKE_FAULTS.md):
    none                          spike padrão
    faulty      INSTR             --faulty-instruction INSTR
    illegal     INSTR             --illegal-instruction INSTR
    RAW         VREG              --RAW-register VREG (hex: 10 = v16)
    ndb         CHANCE FAULT      --ndb-chance CHANCE --ndb-fault FAULT
    cold_start  N INSTR           --cold-start=N,INSTR
    good_start  N INSTR           --good-start=N,INSTR
    three       I1 I2 I3          --three-instruction-err=I1,I2,I3
    four        I1 I2 I3 I4       --four-instruction-err=I1,I2,I3,I4

INSTR pode ser o hexadecimal (0x02113057) ou o nome do define em
benchmarks/common/asm_functions.h (VADD_VI_INSTR, VADD_VI ou vadd.vi).

Exemplos:
    python3 test_runs/run_kernels_faulty.py faulty vadd.vi
    python3 test_runs/run_kernels_faulty.py cold_start 5 0x02840457 -k lmul,vlen
    python3 test_runs/run_kernels_faulty.py ndb 0.01 2 --compare -q
"""

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

ISA = "rv64gcv_zvl128b_zicntr_zba_zbb"
ASM_FUNCTIONS = Path("./benchmarks/common/asm_functions.h")

KERNELS = [
    "mask_agnostic",
    "tail_agnostic",
    "vector_compare",
    "vlen",
    "lmul",
    "data_hazards",
    "interruption",
]

# Linhas ignoradas ao comparar com a execução do spike padrão
IGNORE_IN_DIFF = re.compile(r"<SPIKE_DMG>|^ALO$|Cycles:")

# Os kernels geralmente retornam 0 mesmo quando falham; procura as mensagens
FAIL_PATTERN = re.compile(r"fail|mismatch", re.IGNORECASE)


def load_instructions():
    """Lê os #define *_INSTR de asm_functions.h -> {NOME: hex}."""
    table = {}
    if not ASM_FUNCTIONS.exists():
        return table
    pattern = re.compile(r"#define\s+(\w+)_INSTR\s+(0x[0-9a-fA-F]+)")
    for line in ASM_FUNCTIONS.read_text().splitlines():
        m = pattern.search(line)
        if m:
            table[m.group(1).upper()] = m.group(2).lower()
    return table


def resolve_instr(value, table):
    """Converte nome de instrução (VADD_VI_INSTR / VADD_VI / vadd.vi) ou hex em hex."""
    if re.fullmatch(r"(0x)?[0-9a-fA-F]+", value):
        return value if value.lower().startswith("0x") else f"0x{value}"

    key = value.upper().replace(".", "_").removesuffix("_INSTR")
    if key not in table:
        sys.exit(f"Instrução desconhecida: {value} (veja {ASM_FUNCTIONS})")
    return table[key]


def build_fault_args(mode, values, table):
    expected = {
        "none": 0, "faulty": 1, "illegal": 1, "RAW": 1, "ndb": 2,
        "cold_start": 2, "good_start": 2, "three": 3, "four": 4,
    }
    if len(values) != expected[mode]:
        sys.exit(f"Modo '{mode}' espera {expected[mode]} valor(es), recebeu {len(values)}")

    if mode == "none":
        return []
    if mode == "faulty":
        return ["--faulty-instruction", resolve_instr(values[0], table)]
    if mode == "illegal":
        return ["--illegal-instruction", resolve_instr(values[0], table)]
    if mode == "RAW":
        return ["--RAW-register", values[0]]
    if mode == "ndb":
        return ["--ndb-chance", values[0], "--ndb-fault", values[1]]
    if mode == "cold_start":
        return [f"--cold-start={values[0]},{resolve_instr(values[1], table)}"]
    if mode == "good_start":
        return [f"--good-start={values[0]},{resolve_instr(values[1], table)}"]
    instrs = ",".join(resolve_instr(v, table) for v in values)
    if mode == "three":
        return [f"--three-instruction-err={instrs}"]
    return [f"--four-instruction-err={instrs}"]


def run_spike(spike, fault_args, kernel, timeout):
    cmd = [spike, f"--isa={ISA}", *fault_args, f"./benchmarks/test_{kernel}.riscv"]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        return p.returncode, p.stdout + p.stderr
    except subprocess.TimeoutExpired as e:
        out = (e.stdout or b"").decode(errors="replace") if isinstance(e.stdout, bytes) else (e.stdout or "")
        return "timeout", out


def filtered(output):
    return [l for l in output.splitlines() if not IGNORE_IN_DIFF.search(l)]


def main():
    parser = argparse.ArgumentParser(
        description="Roda os testes kernels com o Spike modificado.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("mode", choices=["none", "faulty", "illegal", "RAW", "ndb",
                                         "cold_start", "good_start", "three", "four"])
    parser.add_argument("values", nargs="*", help="parâmetros do modo")
    parser.add_argument("-k", "--kernels", default=",".join(KERNELS),
                        help="lista separada por vírgula (padrão: todos)")
    parser.add_argument("--spike", default="spike", help="binário do spike (padrão: spike)")
    parser.add_argument("--timeout", type=float, default=60, help="segundos por kernel")
    parser.add_argument("--compare", action="store_true",
                        help="roda também o spike padrão e compara as saídas")
    parser.add_argument("-o", "--out-dir", help="salva a saída de cada kernel neste diretório")
    parser.add_argument("-q", "--quiet", action="store_true", help="mostra só o resumo")
    args = parser.parse_args()

    # Resolve antes do chdir para que -o seja relativo ao diretório de chamada
    out_dir = Path(args.out_dir).resolve() if args.out_dir else None

    if Path.cwd().name == "test_runs":
        os.chdir("..")

    table = load_instructions()
    fault_args = build_fault_args(args.mode, args.values, table)
    kernels = [k.strip().removeprefix("test_") for k in args.kernels.split(",") if k.strip()]

    if out_dir:
        out_dir.mkdir(parents=True, exist_ok=True)

    print(f"spike {' '.join(fault_args) or '(padrão)'}")

    summary = []
    for kernel in kernels:
        if not Path(f"./benchmarks/test_{kernel}.riscv").exists():
            print(f"Aviso: ./benchmarks/test_{kernel}.riscv não encontrado. Pulando...")
            continue

        rc, output = run_spike(args.spike, fault_args, kernel, args.timeout)

        if not args.quiet:
            print(f"\n=== test_{kernel} ===")
            print(output, end="" if output.endswith("\n") else "\n")

        if out_dir:
            (out_dir / f"test_{kernel}.{args.mode}.out").write_text(output)

        diff = "-"
        if args.compare:
            golden_rc, golden = run_spike(args.spike, [], kernel, args.timeout)
            same = rc == golden_rc and filtered(output) == filtered(golden)
            diff = "igual" if same else "DIFERENTE"

        failed = rc != 0 or bool(FAIL_PATTERN.search(output))
        summary.append((kernel, rc, "sim" if failed else "não", diff))

    print("\n=== Resumo ===")
    header = f"{'kernel':<16} {'rc':>8}  {'falhou':<7}"
    if args.compare:
        header += "  vs. padrão"
    print(header)
    for kernel, rc, failed, diff in summary:
        line = f"{kernel:<16} {str(rc):>8}  {failed:<7}"
        if args.compare:
            line += f"  {diff}"
        print(line)


if __name__ == "__main__":
    main()
