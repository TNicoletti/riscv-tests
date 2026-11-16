#!/usr/bin/env python3
"""
Analisa as saídas do multi_run_all_instructions.py (M1-M6 no test_all_instructions):

    PAPER/<M>/<falha>.out      PAPER/<M>/manifest.csv      PAPER/golden/golden.out

O test_all_instructions roda cada instrução sozinha `repeat_instruction` vezes e imprime,
por instrução, o resultado de cada repetição e PASS/ERR:

    0 - VADD_VV       [XX] [WA] [II] ... ERR
    XX certo, WA resposta errada, II instrução ilegal, ... (get_err em add_instruction.c)

Ele não minimiza nem rotula a falha; por isso as perguntas são:

    detectou   alguma instrução deu ERR (ou o teste de vle32/vse32 falhou, ou o programa
               não terminou)
    localizou  as instruções com ERR são exatamente as da falha injetada
    padrão     o padrão de [..] da instrução bate com o modelo:
                 M1 só [II]; M2 só [WA]; M4 erros esparsos (instrução com ERR que também
                 acerta em alguma repetição); M6 erros só nas primeiras 5 execuções
    "-" = não se aplica (M3 e M5 não apontam instrução/padrão num teste de instrução isolada;
          M4 não tem instrução alvo). Os "-" não entram nas porcentagens.

    python3 test_runs/analyze_all_instructions.py [PAPER_DIR] [-e M1,M6] [--list] [--export]

--list   mostra, por falha, as instruções com ERR e os resultados de cada repetição
--export grava PAPER/<M>/analysis.csv (uma linha por falha) e PAPER/summary.csv
"""

import argparse
import csv
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

DEFAULT_PAPER_DIR = Path("./benchmarks/test_all_instructions/test_runs/paper")
EXPERIMENTS = ["M1", "M2", "M3", "M4", "M5", "M6"]
TITLES = {
    "M1": "Illegal instruction",
    "M2": "Wrong result",
    "M3": "RAW hazard",
    "M4": "Intermittent",
    "M5": "Sequence",
    "M6": "Cold start",
}
COLD_START_N = 5

RESULT_PATTERN = re.compile(r"^\d+ - (\S+)\s+((?:\[\w+\]\s*)+)(PASS|ERR)\s*$", re.M)
LS_PATTERN = re.compile(r"^VLE32_V VSE32_V\s+((?:\[\w+\]\s*)+)$", re.M)
CODE_PATTERN = re.compile(r"\[(\w+)\]")
FINISHED_PATTERN = re.compile(r"Correct instructions / total: (\d+) / (\d+)")

# Instruções que o Spike não distingue (mesmo funct6|funct3|opcode)
ALIASES = {"vfirst_m": "vcpop_m"}


def canonical(name):
    name = name.lower()
    return ALIASES.get(name, name)


@dataclass
class Output:
    path: Path
    finished: bool = False
    ls_ok: bool = True                              # vle32/vse32 todos [XX]
    errors: dict = field(default_factory=dict)      # instrução (minúscula) -> [códigos]
    executions: int = 0                             # repetições de todas as instruções
    wrong: int = 0                                  # repetições != XX

    @property
    def detected(self):
        return bool(self.errors) or not self.ls_ok or not self.finished

    @property
    def error_set(self):
        return {canonical(n) for n in self.errors}


def parse_output(path):
    text = path.read_text(encoding="utf-8", errors="ignore")
    out = Output(path, finished=bool(FINISHED_PATTERN.search(text)))
    m = LS_PATTERN.search(text)
    if m:
        out.ls_ok = all(c == "XX" for c in CODE_PATTERN.findall(m.group(1)))
    for name, codes, status in RESULT_PATTERN.findall(text):
        codes = CODE_PATTERN.findall(codes)
        out.executions += len(codes)
        out.wrong += sum(c != "XX" for c in codes)
        if status == "ERR":
            out.errors[name.lower()] = codes
    return out


# ---------------------------------------------------------------------------
# Critérios por modelo: (localizou, padrão); None = não se aplica

def sequence(fault):
    # three00_vmv_v_v-vor_vv-vand_vx -> [vmv_v_v, vor_vv, vand_vx]
    return fault.split("_", 1)[1].split("-")


def target_codes(fault, out):
    """Códigos [..] da instrução alvo (ou do alias que o Spike confunde com ela)."""
    for name, codes in out.errors.items():
        if canonical(name) == canonical(fault):
            return codes
    return []


def only_target(fault, out):
    return out.ls_ok and out.error_set == {canonical(fault)}


def criteria(experiment, fault, out):
    if experiment == "M1":
        codes = target_codes(fault, out)
        return only_target(fault, out), bool(codes) and all(c == "II" for c in codes)
    if experiment == "M2":
        codes = [c for c in target_codes(fault, out) if c != "XX"]
        return only_target(fault, out), bool(codes) and all(c == "WA" for c in codes)
    if experiment == "M4":
        sparse = any("XX" in codes for codes in out.errors.values())
        return None, out.detected and sparse
    if experiment == "M5":
        seq = {canonical(n) for n in sequence(fault)}
        return (bool(out.error_set) and out.ls_ok and out.error_set <= seq), None
    if experiment == "M6":
        codes = target_codes(fault, out)
        wrong_at = [i for i, c in enumerate(codes) if c != "XX"]
        cold = bool(wrong_at) and max(wrong_at) < COLD_START_N
        return only_target(fault, out), cold
    return None, None  # M3


def note(experiment, fault, out):
    if experiment == "M4" and out.executions:
        p = float(fault.split("_")[0][1:])
        return f"p={p:g} p̂={out.wrong / out.executions:.3f}"
    return ""


def groups(experiment, results):
    """M5 sai separado em triplas e quádruplas."""
    if experiment == "M5":
        return [("Sequence triplet", [r for r in results if r.fault.startswith("three")]),
                ("Sequence quadruplet", [r for r in results if r.fault.startswith("four")])]
    return [(TITLES[experiment], results)]


# ---------------------------------------------------------------------------

@dataclass
class Result:
    experiment: str
    fault: str
    out: Output
    detected: bool
    localized: bool | None
    labeled: bool | None
    seconds: float | None
    note: str


def load_times(exp_dir):
    times = {}
    manifest = exp_dir / "manifest.csv"
    if manifest.is_file():
        with open(manifest, newline="") as f:
            for row in csv.DictReader(f):
                try:
                    times[row["fault"]] = float(row["seconds"])
                except (KeyError, ValueError):
                    pass
    return times


def analyze_experiment(paper_dir, experiment):
    exp_dir = paper_dir / experiment
    if not exp_dir.is_dir():
        return []
    times = load_times(exp_dir)
    results = []
    for path in sorted(exp_dir.glob("*.out")):
        fault = path.stem
        out = parse_output(path)
        localized, labeled = criteria(experiment, fault, out)
        if not out.detected:
            localized = False if localized is not None else None
            labeled = False if labeled is not None else None
        results.append(Result(experiment, fault, out, out.detected, localized, labeled,
                              times.get(fault), note(experiment, fault, out)))
    return results


def pct(k, n):
    return f"{k}/{n} ({100 * k / n:.0f}%)" if n else "-"


def count(results, attr):
    """(acertos, aplicáveis) de um critério que pode ser None."""
    values = [getattr(r, attr) for r in results if getattr(r, attr) is not None]
    return sum(values), len(values)


def cell(results, attr):
    k, n = count(results, attr)
    return pct(k, n) if n else "-"


def flag(value, letter):
    return "-" if value is None else (letter if value else ".")


def print_model(name, results, show_list):
    print("=" * 78)
    print(f" {name}")
    print("=" * 78)
    print("Por falha (D = detectou, L = localizou, P = padrão certo; '-' = não se aplica):")
    width = max(12, max(len(r.fault) for r in results))
    for r in results:
        line = (f"  {r.fault:<{width}}  {flag(r.detected, 'D')}{flag(r.localized, 'L')}"
                f"{flag(r.labeled, 'P')}")
        if r.note:
            line += f"  {r.note}"
        if show_list:
            errs = "; ".join(f"{n.upper()} {' '.join(c)}" for n, c in r.out.errors.items())
            extra = [] if r.out.finished else ["não terminou"]
            extra += [] if r.out.ls_ok else ["vle32/vse32 falhou"]
            if errs or extra:
                line += f"  | {', '.join(extra + ([errs] if errs else []))}"
        print(line)
    secs = [r.seconds for r in results if r.seconds is not None]
    print()
    print(f"{'N':>4} {'detectou':>16} {'localizou':>16} {'padrão':>16} {'tempo médio':>12}")
    print(f"{len(results):>4} {cell(results, 'detected'):>16} {cell(results, 'localized'):>16} "
          f"{cell(results, 'labeled'):>16} "
          f"{(f'{sum(secs) / len(secs):.2f}s' if secs else '-'):>12}")
    print()


def main():
    parser = argparse.ArgumentParser(description="Analisa M1-M6 no test_all_instructions.")
    parser.add_argument("paper_dir", nargs="?", default=str(DEFAULT_PAPER_DIR),
                        help=f"pasta de resultados (padrão: {DEFAULT_PAPER_DIR})")
    parser.add_argument("-e", "--experiments", default=",".join(EXPERIMENTS),
                        help="lista separada por vírgula (padrão: todos)")
    parser.add_argument("--list", action="store_true",
                        help="mostra as instruções com ERR de cada falha")
    parser.add_argument("--export", action="store_true",
                        help="grava PAPER/<M>/analysis.csv e PAPER/summary.csv")
    args = parser.parse_args()

    paper_dir = Path(args.paper_dir)
    if not paper_dir.is_dir() and Path.cwd().name == "test_runs":
        paper_dir = Path("..") / paper_dir
    if not paper_dir.is_dir():
        sys.exit(f"Erro: '{paper_dir}' não encontrado.")
    experiments = [e.strip().upper() for e in args.experiments.split(",") if e.strip()]

    rows = []
    summary = []  # (nome, results)
    for experiment in experiments:
        results = analyze_experiment(paper_dir, experiment)
        if not results:
            continue
        for title, group in groups(experiment, results):
            if group:
                print_model(f"{experiment} {title}", group, args.list)
                summary.append((experiment, title, group))
        if args.export:
            path = paper_dir / experiment / "analysis.csv"
            with open(path, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["fault", "detected", "localized", "labeled", "finished",
                            "error_instructions", "seconds", "note"])
                for r in results:
                    w.writerow([r.fault, int(r.detected),
                                "" if r.localized is None else int(r.localized),
                                "" if r.labeled is None else int(r.labeled),
                                int(r.out.finished), " ".join(r.out.errors),
                                "" if r.seconds is None else f"{r.seconds:.2f}", r.note])

    if not summary:
        sys.exit(f"Nenhum experimento encontrado em '{paper_dir}'.")

    # ------------------------------------------------------------------
    print("=" * 96)
    print(" RESUMO (test_all_instructions, uma execução por falha)")
    print("=" * 96)
    print(f"{'modelo':<27} {'N':>4} {'detectou':>16} {'localizou':>16} {'padrão':>16} "
          f"{'tempo médio':>12}")
    all_results = []
    for experiment, title, results in summary:
        all_results += results
        secs = [r.seconds for r in results if r.seconds is not None]
        mean_sec = sum(secs) / len(secs) if secs else None
        print(f"{experiment + ' ' + title:<27} {len(results):>4} {cell(results, 'detected'):>16} "
              f"{cell(results, 'localized'):>16} {cell(results, 'labeled'):>16} "
              f"{(f'{mean_sec:.2f}s' if mean_sec is not None else '-'):>12}")
        rows.append([experiment, title, len(results), *count(results, "detected"),
                     *count(results, "localized"), *count(results, "labeled"),
                     "" if mean_sec is None else f"{mean_sec:.2f}"])
    print(f"{'Total':<27} {len(all_results):>4} {cell(all_results, 'detected'):>16} "
          f"{cell(all_results, 'localized'):>16} {cell(all_results, 'labeled'):>16}")
    rows.append(["Total", "", len(all_results), *count(all_results, "detected"),
                 *count(all_results, "localized"), *count(all_results, "labeled"), ""])
    print("  localizou/padrão contam só as falhas em que o critério se aplica")

    golden = paper_dir / "golden" / "golden.out"
    if golden.is_file():
        g = parse_output(golden)
        status = "nenhum" if not g.detected else (
            "não terminou" if not g.finished else
            ", ".join(n.upper() for n in g.errors) or "vle32/vse32 falhou")
        print(f"\nFalsos positivos no Spike padrão (golden): {status}")

    print("\nFalhas não detectadas:")
    any_missing = False
    for experiment, title, results in summary:
        missing = [r.fault for r in results if not r.detected]
        if missing:
            any_missing = True
            print(f"  {experiment} {title} ({len(missing)}): {' '.join(missing)}")
    if not any_missing:
        print("  (nenhuma)")

    wrong_place = [r for r in all_results if r.detected and r.localized is False]
    if wrong_place:
        print("\nDetectadas mas com ERR fora da instrução injetada:")
        for r in wrong_place:
            print(f"  {r.experiment} {r.fault}: {' '.join(n.upper() for n in r.out.errors) or '-'}")

    if args.export:
        path = paper_dir / "summary.csv"
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["experiment", "model", "faults", "detected", "detected_applicable",
                        "localized", "localized_applicable", "labeled", "labeled_applicable",
                        "mean_seconds"])
            w.writerows(rows)
        print(f"\nCSV exportado para: {path} (e <M>/analysis.csv)")


if __name__ == "__main__":
    main()
