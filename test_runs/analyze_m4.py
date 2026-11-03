#!/usr/bin/env python3
"""
Analisa o M4 (intermittent): cada instrução vetorial falha com probabilidade p
(--ndb-chance p --ndb-fault 1). O ndb não escolhe a instrução.

    python3 test_runs/analyze_m4.py [PAPER_DIR] [--list] [--export]

Critérios:
    detectou   o programa encontrou a divergência
    minimizou  "Minimized instruction set" tem 1 instrução (qualquer uma, já que o ndb age
               em todas)
    rotulou    "Error is not compulsory" e "Non deterministic random behavior detected"
               (não pode ser rotulado como cold start)

Com --list mostra também p injetado e o "Chance of instruction been wrong" estimado.
"""

from paper_analysis import main_for

EXPERIMENT = "M4"
TITLE = "Intermittent"
CRITERIA = [
    "detectou:  o programa encontrou a divergência",
    "minimizou: o conjunto minimizado tem 1 instrução (o ndb não escolhe instrução)",
    "rotulou:   não determinística (Error is not compulsory + Non deterministic random behavior)",
]


def injected_chance(fault):
    return float(fault.split("_")[0].removeprefix("p"))


def minimized(fault, out):
    return len(out.minimized) == 1


def labeled(fault, out):
    return out.compulsory is False and out.non_deterministic and not out.cold_start


def note(fault, out):
    estimate = "-" if out.chance is None else f"{out.chance:g}%"
    state = "" if out.finished else " (diagnóstico incompleto)"
    return f"p={100 * injected_chance(fault):g}% estimado={estimate}{state}"


if __name__ == "__main__":
    main_for(EXPERIMENT, TITLE, CRITERIA, minimized, labeled, note)
