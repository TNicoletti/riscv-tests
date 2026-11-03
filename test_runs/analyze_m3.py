#!/usr/bin/env python3
"""
Analisa o M3 (RAW hazard): trap quando uma instrução vetorial lê vR logo depois de outra
instrução vetorial escrever em vR (--RAW-register R).

    python3 test_runs/analyze_m3.py [PAPER_DIR] [--list] [--export]

Critérios:
    detectou   o programa encontrou a divergência
    minimizou  "Minimized instruction set" tem 2 instruções: a primeira escreve em vR e a
               segunda lê vR
    rotulou    "Error is compulsory" (determinística) e algum rótulo de hazard ou registrador:
               "Possible Read after Write", "related to data hazards" (NOPs resolvem) ou
               "related to registers"
"""

from paper_analysis import main_for

EXPERIMENT = "M3"
TITLE = "RAW hazard"
CRITERIA = [
    "detectou:  o programa encontrou a divergência",
    "minimizou: 2 instruções minimizadas, a 1ª escreve vR e a 2ª lê vR",
    "rotulou:   determinística e RAW / NOPs resolvem / trocar registradores resolve",
]


def register(fault):
    return int(fault.removeprefix("v"))


def minimized(fault, out):
    if len(out.minimized) != 2:
        return False
    reg = register(fault)
    (_, vd, _, _), (_, _, vs2, vs1) = out.minimized
    return vd == reg and reg in (vs2, vs1)


def labeled(fault, out):
    return out.compulsory is True and (out.raw or out.nops_solve or out.registers)


def note(fault, out):
    if not out.finished:
        return "diagnóstico incompleto (timeout?)"
    insns = " | ".join(f"{n} v{a},v{b},v{c}" for n, a, b, c in out.minimized)
    tags = [t for t, ok in (("RAW", out.raw), ("NOPs", out.nops_solve), ("regs", out.registers)) if ok]
    return f"{insns or '-'} [{','.join(tags)}]"


if __name__ == "__main__":
    main_for(EXPERIMENT, TITLE, CRITERIA, minimized, labeled, note)
