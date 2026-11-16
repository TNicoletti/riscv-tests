#!/usr/bin/env python3
"""
Funções comuns de analyze_kernels.py e analyze_kernel.py. Lê as saídas do multi_run_kernels.py:

    KPAPER/<M>/<falha>/<kernel>.out     KPAPER/<M>/manifest.csv
    KPAPER/golden/golden/<kernel>.out

Cada execução (falha x kernel) recebe um status:

    crash   o programa não terminou normalmente: rc != 0, timeout ou "*** FAILED ***"
            (ex.: instrução ilegal sem tratamento -> tohost = 57)
    check   o próprio kernel acusou erro ([FAIL], Mismatch, "Vectors were not equal",
            "ERROR: ...", "test fail", "... error on should compute values", "x != y")
    diff    o kernel não acusou nada, mas a saída é diferente da do Spike padrão
            (ignorando ciclos/IPC/speedup e as mensagens <SPIKE_DMG>)
    ok      saída igual à do Spike padrão

"Detectou" = crash ou check (o kernel falhou sozinho, sem precisar de referência).
"diff" aparece à parte: só seria percebido comparando com uma execução correta.
"""

import csv
import re
from dataclasses import dataclass
from pathlib import Path

DEFAULT_KPAPER_DIR = Path("./test_runs/kernels/paper")
EXPERIMENTS = ["M1", "M2", "M3", "M4", "M5", "M6"]
TITLES = {
    "M1": "Illegal instruction",
    "M2": "Wrong result",
    "M3": "RAW hazard",
    "M4": "Intermittent",
    "M5": "Sequence",
    "M6": "Cold start",
}
KERNELS = ["mask_agnostic", "tail_agnostic", "vector_compare", "vlen", "lmul",
           "data_hazards", "interruption"]
SHORT = {"mask_agnostic": "mask", "tail_agnostic": "tail", "vector_compare": "vcomp",
         "vlen": "vlen", "lmul": "lmul", "data_hazards": "hazard", "interruption": "intr"}

IGNORE = re.compile(r"<SPIKE_DMG>|^ALO$|Cycles|Ciclos|IPC|Speedup")
CRASH_PATTERN = re.compile(r"\*\*\* FAILED \*\*\*")
CHECK_PATTERN = re.compile(
    r"\[FAIL\]|Mismatch at|Vectors were not equal|^ERROR:|test fail|"
    r"error on should compute values|should remain|^\[\d+\]: -?\d+ != -?\d+", re.M)
HEADER_PATTERN = re.compile(r"^(\[TEST\] .*?)(?: \(Immediate.*)?$|^=====+ (.*?) =+$|^--- (.*) ---$")
STATUS_ORDER = {"crash": 0, "check": 1, "diff": 2, "ok": 3}


def relevant_lines(text):
    return [l.rstrip() for l in text.splitlines() if l.strip() and not IGNORE.search(l)]


@dataclass
class Run:
    experiment: str
    fault: str
    kernel: str
    status: str            # crash / check / diff / ok
    rc: object
    seconds: float | None
    where: str             # seção ([TEST] / =====) em que o kernel acusou ou travou

    @property
    def detected(self):
        return self.status in ("crash", "check")


def where_failed(lines):
    """Último cabeçalho antes da primeira falha (ou do fim, se travou)."""
    header = ""
    for line in lines:
        m = HEADER_PATTERN.match(line)
        if m:
            header = next(g for g in m.groups() if g)
        if CHECK_PATTERN.search(line) or CRASH_PATTERN.search(line):
            return header or line.strip()[:60]
    return header


def classify(text, rc, golden_lines):
    lines = relevant_lines(text)
    if rc not in (0, "0", None) or CRASH_PATTERN.search(text):
        return "crash", where_failed(lines)
    if CHECK_PATTERN.search(text):
        return "check", where_failed(lines)
    if golden_lines is not None and lines != golden_lines:
        return "diff", ""
    return "ok", ""


def load_manifest(exp_dir):
    info = {}
    manifest = exp_dir / "manifest.csv"
    if manifest.is_file():
        with open(manifest, newline="") as f:
            for row in csv.DictReader(f):
                rc = row.get("return_code", "")
                try:
                    rc = int(rc)
                except ValueError:
                    pass
                try:
                    seconds = float(row["seconds"])
                except (KeyError, ValueError):
                    seconds = None
                info[(row["fault"], row["kernel"])] = (rc, seconds)
    return info


def load_golden(kpaper_dir):
    golden = {}
    gdir = Path(kpaper_dir) / "golden" / "golden"
    for kernel in KERNELS:
        path = gdir / f"{kernel}.out"
        if path.is_file():
            golden[kernel] = relevant_lines(path.read_text(encoding="utf-8", errors="ignore"))
    return golden


def analyze_experiment(kpaper_dir, experiment, golden, kernels=None):
    """Lista de Run de todas as falhas x kernels do experimento."""
    exp_dir = Path(kpaper_dir) / experiment
    if not exp_dir.is_dir():
        return []
    info = load_manifest(exp_dir)
    runs = []
    for fault_dir in sorted(p for p in exp_dir.iterdir() if p.is_dir()):
        for path in sorted(fault_dir.glob("*.out")):
            kernel = path.stem
            if kernels and kernel not in kernels:
                continue
            rc, seconds = info.get((fault_dir.name, kernel), (None, None))
            text = path.read_text(encoding="utf-8", errors="ignore")
            status, where = classify(text, rc, golden.get(kernel))
            runs.append(Run(experiment, fault_dir.name, kernel, status, rc, seconds, where))
    return runs


def by_fault(runs):
    """{falha: {kernel: Run}} preservando a ordem."""
    faults = {}
    for r in runs:
        faults.setdefault(r.fault, {})[r.kernel] = r
    return faults


def groups(experiment, items, fault_of=lambda x: x):
    """M5 separado em triplas e quádruplas; os outros num grupo só."""
    if experiment == "M5":
        return [("Sequence triplet", [i for i in items if fault_of(i).startswith("three")]),
                ("Sequence quadruplet", [i for i in items if fault_of(i).startswith("four")])]
    return [(TITLES[experiment], list(items))]


def pct(k, n):
    return f"{k}/{n} ({100 * k / n:.0f}%)" if n else "-"


def resolve_dir(path):
    path = Path(path)
    if not path.is_dir() and Path.cwd().name == "test_runs":
        path = Path("..") / path
    return path
