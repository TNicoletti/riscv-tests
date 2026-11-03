#!/usr/bin/env python3
"""
Funções comuns dos analisadores dos experimentos do artigo (analyze_m1.py .. analyze_m6.py
e analyze_summary.py). Lê as saídas do multi_run_paper.py:

    PAPER/<M>/<falha>/<seed>/<entrada>.out

Para cada seed de cada falha, olha o primeiro arquivo que detectou a falha (o multi_run_paper.py
para a seed nele) e responde três perguntas:

    detectou   o programa encontrou a divergência ("Going to error discoverer" / "Failed on test")
    minimizou  o "Minimized instruction set" é exatamente o gatilho da falha injetada
    rotulou    o diagnóstico textual bate com o rótulo esperado do modelo (Tabela II)

Também calcula quantas sequências foram executadas até detectar (arquivos anteriores x 1332 +
teste da "Divergence" + 1) e o tempo da seed (soma do "seconds" do manifest.csv).

Os critérios de "minimizou" e "rotulou" de cada modelo estão nos analyze_m*.py.
"""

import argparse
import csv
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

DEFAULT_PAPER_DIR = Path("./benchmarks/random_neural_network/test_runs/paper")
TESTS_PER_FILE = 1332

DETECTED_PATTERN = re.compile(r"Going to error discoverer|==== Failed on test \d+ ====")
FINISHED_PATTERN = re.compile(r"==== Failed on test (\d+) ====")
MINIMIZED_PATTERN = re.compile(r"Minimized instruction set with (\d+) instructions:")
INSTRUCTION_PATTERN = re.compile(r"^\s*([A-Z][A-Z0-9_]*)\s+v(\d+),\s*v(\d+),\s*v(\d+)")
CHANCE_PATTERN = re.compile(r"Chance of instruction been wrong:\s*(\d+(?:\.\d+)?)%")
SINGLE_PATTERN = re.compile(r"Probably problematic instruction:\s*(\S+)")
DIVERGENCE_PATTERN = re.compile(r"Divergence (\d+)-(\d+)")

# Instruções que o Spike não distingue (mesmo funct6|funct3|opcode)
ALIASES = {"vcpop_m": "vcpop_m", "vfirst_m": "vcpop_m"}


def same_instruction(a, b):
    return ALIASES.get(a, a) == ALIASES.get(b, b)


@dataclass
class Output:
    """O que foi extraído de um .out."""
    path: Path
    detected: bool = False
    test_index: int | None = None       # teste do arquivo em que apareceu a divergência
    finished: bool = False              # chegou ao "Failed on test" (diagnóstico completo)
    failed_test: int | None = None
    compulsory: bool | None = None      # True: "Error is compulsory"; False: "not compulsory"
    cold_start: bool = False
    non_deterministic: bool = False
    nops_solve: bool = False            # heurística 1: NOPs entre instruções resolvem
    single_instruction: bool = False    # heurística 2: problema em uma instrução só
    multiple_instructions: bool = False # heurística 2: interferência entre instruções
    registers: bool = False             # heurística 4: trocar registradores resolve
    raw: bool = False
    waw: bool = False
    chance: float | None = None
    problematic: str | None = None      # instrução apontada pela heurística 2
    minimized: list = field(default_factory=list)  # [(nome, vd, vs2, vs1)]

    @property
    def minimized_names(self):
        return [name for name, *_ in self.minimized]


def parse_output(path):
    text = path.read_text(encoding="utf-8", errors="ignore")
    out = Output(path)
    out.detected = bool(DETECTED_PATTERN.search(text))
    if not out.detected:
        return out

    m = DIVERGENCE_PATTERN.search(text)
    if m:
        start, end = int(m.group(1)), int(m.group(2))
        out.test_index = start // (end - start)

    m = FINISHED_PATTERN.search(text)
    out.finished = bool(m)
    out.failed_test = int(m.group(1)) if m else None

    if "Error is compulsory" in text:
        out.compulsory = True
    elif "Error is not compulsory" in text:
        out.compulsory = False
    out.cold_start = "Could be cold start problem" in text
    out.non_deterministic = "Non deterministic random behavior detected" in text
    out.nops_solve = "Problem is probably related to data hazards" in text
    out.single_instruction = "Problem is probably related to a single instruction" in text
    out.multiple_instructions = ("Problem is probably related to multiple instructions interference" in text
                                 or "Problem with multiple instructions" in text)
    out.registers = "Problem is related to registers" in text
    out.raw = "Possible Read after Write data hazard detected" in text
    out.waw = "Possible Write after Write data hazard detected" in text

    m = CHANCE_PATTERN.search(text)
    out.chance = float(m.group(1)) if m else None
    m = SINGLE_PATTERN.search(text)
    out.problematic = m.group(1).lower() if m else None

    m = MINIMIZED_PATTERN.search(text)
    if m:
        count = int(m.group(1))
        lines = text[m.end():].splitlines()[1:]
        for line in lines[:count]:
            im = INSTRUCTION_PATTERN.match(line)
            if im:
                out.minimized.append((im.group(1).lower(), int(im.group(2)),
                                      int(im.group(3)), int(im.group(4))))
    return out


@dataclass
class SeedResult:
    fault: str
    seed: str
    files_run: int
    detected_by: str = ""
    detected: bool = False
    minimized: bool = False
    labeled: bool = False
    finished: bool = False
    sequences: int | None = None    # sequências executadas até detectar
    seconds: float | None = None    # tempo da seed (manifest.csv)
    note: str = ""          # detalhe do diagnóstico (instruções minimizadas, p estimado...)


def analyze_seed(fault, seed_dir, minimized_fn, labeled_fn, note_fn, times=None):
    """Acha o primeiro arquivo da seed que detectou e aplica os critérios do modelo.

    times: {(falha, seed, arquivo .out): segundos}, do manifest.csv"""
    files = sorted(seed_dir.glob("*.out"))
    result = SeedResult(fault, seed_dir.name, len(files))
    for i, path in enumerate(files):
        out = parse_output(path)
        if out.detected:
            result.files_run = i + 1
            result.detected_by = path.name
            result.detected = True
            result.finished = out.finished
            if out.test_index is not None:
                result.sequences = i * TESTS_PER_FILE + out.test_index + 1
            result.minimized = bool(minimized_fn(fault, out))
            result.labeled = bool(labeled_fn(fault, out))
            result.note = note_fn(fault, out)
            break
    if times:
        seconds = [times.get((fault, seed_dir.name, f.name)) for f in files[:result.files_run]]
        if seconds and None not in seconds:
            result.seconds = sum(seconds)
    return result


def load_times(exp_dir):
    """{(falha, seed, arquivo .out): segundos} do manifest.csv do experimento."""
    times = {}
    manifest = exp_dir / "manifest.csv"
    if not manifest.exists():
        return times
    with open(manifest, newline="") as f:
        for r in csv.DictReader(f):
            try:
                times[(r["fault"], r["seed"], r["input"].removesuffix(".in") + ".out")] = float(r["seconds"])
            except (KeyError, ValueError):
                pass
    return times


def default_note(fault, out):
    if not out.finished:
        return "diagnóstico incompleto (timeout?)"
    return " ".join(out.minimized_names) or "-"


def analyze_experiment(paper_dir, experiment, minimized_fn, labeled_fn, note_fn=default_note):
    """Lista de SeedResult de todas as falhas e seeds do experimento."""
    exp_dir = Path(paper_dir) / experiment
    if not exp_dir.is_dir():
        return []
    times = load_times(exp_dir)
    results = []
    for fault_dir in sorted(p for p in exp_dir.iterdir() if p.is_dir()):
        for seed_dir in sorted(p for p in fault_dir.iterdir() if p.is_dir()):
            results.append(analyze_seed(fault_dir.name, seed_dir, minimized_fn, labeled_fn, note_fn,
                                        times))
    return results


def cost_summary(results):
    """{seed: (média de sequências até detectar, mediana, tempo médio da seed, tempo total)}"""
    summary = {}
    for seed in sorted({r.seed for r in results}):
        rs = [r for r in results if r.seed == seed]
        seqs = sorted(r.sequences for r in rs if r.sequences is not None)
        secs = [r.seconds for r in rs if r.seconds is not None]
        summary[seed] = (
            sum(seqs) / len(seqs) if seqs else None,
            seqs[len(seqs) // 2] if seqs else None,
            sum(secs) / len(secs) if secs else None,
            sum(secs) if secs else None,
        )
    return summary


def fmt(value, spec, suffix=""):
    return "-" if value is None else f"{value:{spec}}{suffix}"


def seed_summary(results):
    """{seed: (total, detectou, minimizou, rotulou)}"""
    summary = {}
    for r in results:
        t, d, m, l = summary.get(r.seed, (0, 0, 0, 0))
        summary[r.seed] = (t + 1, d + r.detected, m + r.minimized, l + r.labeled)
    return dict(sorted(summary.items()))


def pct(n, total):
    return f"{n}/{total} ({100 * n / total:.0f}%)" if total else "-"


def flags(r):
    return ("D" if r.detected else "-") + ("M" if r.minimized else "-") + ("L" if r.labeled else "-")


def print_report(title, results, criteria, show_list=False):
    print("=" * 78)
    print(f" {title}")
    print("=" * 78)
    if not results:
        print("Nenhum resultado encontrado.")
        return

    print("Critérios:")
    for line in criteria:
        print(f"  {line}")
    print()

    seeds = sorted({r.seed for r in results})
    faults = sorted({r.fault for r in results}, key=lambda f: [r.fault for r in results].index(f))
    by_key = {(r.fault, r.seed): r for r in results}
    width = max(12, max(len(f) for f in faults))

    print("Por falha (D = detectou, M = minimizou, L = rotulou certo; n = arquivos rodados):")
    print(f"{'falha':<{width}}  " + "  ".join(f"{s:>10}" for s in seeds))
    for fault in faults:
        cells = []
        for s in seeds:
            r = by_key.get((fault, s))
            cells.append(f"{flags(r)} {r.files_run:>3}".rjust(10) if r else " " * 10)
        line = f"{fault:<{width}}  " + "  ".join(cells)
        if show_list:
            notes = "; ".join(f"{s}: {by_key[(fault, s)].note}" for s in seeds
                              if (fault, s) in by_key and by_key[(fault, s)].detected)
            line += f"   {notes}"
        print(line)
    print()

    print("Por seed (seq. = sequências executadas até detectar, só nas seeds que detectaram;")
    print("          tempo = tempo de uma seed, incluindo as que não detectaram):")
    print(f"{'seed':<6} {'detectou':>16} {'minimizou':>16} {'rotulou':>16} "
          f"{'seq. média':>11} {'seq. mediana':>13} {'tempo médio':>12} {'tempo total':>12}")
    costs = cost_summary(results)
    for s, (t, d, m, l) in seed_summary(results).items():
        mean_seq, median_seq, mean_sec, total_sec = costs[s]
        print(f"{s:<6} {pct(d, t):>16} {pct(m, t):>16} {pct(l, t):>16} "
              f"{fmt(mean_seq, '.0f'):>11} {fmt(median_seq, 'd'):>13} "
              f"{fmt(mean_sec, '.2f', 's'):>12} {fmt(total_sec, '.1f', 's'):>12}")

    t = len(faults)
    any_d = sum(any(by_key[(f, s)].detected for s in seeds if (f, s) in by_key) for f in faults)
    any_m = sum(any(by_key[(f, s)].minimized for s in seeds if (f, s) in by_key) for f in faults)
    any_l = sum(any(by_key[(f, s)].labeled for s in seeds if (f, s) in by_key) for f in faults)
    print(f"{'alguma':<6} {pct(any_d, t):>16} {pct(any_m, t):>16} {pct(any_l, t):>16}")
    incomplete = sum(r.detected and not r.finished for r in results)
    if incomplete:
        print(f"\n{incomplete} seed(s) detectaram mas o diagnóstico não terminou (timeout); contam como não minimizadas.")


def export_csv(results, path):
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["fault", "seed", "detected", "minimized", "labeled", "finished",
                    "files_run", "detected_by", "sequences", "seconds", "note"])
        for r in results:
            w.writerow([r.fault, r.seed, int(r.detected), int(r.minimized), int(r.labeled),
                        int(r.finished), r.files_run, r.detected_by,
                        "" if r.sequences is None else r.sequences,
                        "" if r.seconds is None else f"{r.seconds:.2f}", r.note])
    print(f"\nCSV exportado para: {path}")


def main_for(experiment, title, criteria, minimized_fn, labeled_fn, note_fn=default_note):
    """main() comum dos analyze_m*.py."""
    parser = argparse.ArgumentParser(description=f"Analisa o experimento {experiment} ({title}).")
    parser.add_argument("paper_dir", nargs="?", default=str(DEFAULT_PAPER_DIR),
                        help=f"pasta de resultados do multi_run_paper.py (padrão: {DEFAULT_PAPER_DIR})")
    parser.add_argument("--list", action="store_true",
                        help="mostra o que foi minimizado/estimado em cada seed")
    parser.add_argument("--export", action="store_true",
                        help="grava PAPER/<M>/analysis.csv")
    args = parser.parse_args()

    paper_dir = Path(args.paper_dir)
    if not paper_dir.is_dir() and Path.cwd().name == "test_runs":
        paper_dir = Path("..") / paper_dir
    if not (paper_dir / experiment).is_dir():
        sys.exit(f"Erro: '{paper_dir / experiment}' não encontrado.")

    results = analyze_experiment(paper_dir, experiment, minimized_fn, labeled_fn, note_fn)
    print_report(f"{experiment} {title}", results, criteria, args.list)
    if args.export:
        export_csv(results, paper_dir / experiment / "analysis.csv")
    return results
