#!/usr/bin/env python3
"""
Analisa o M6 (cold start): as 5 primeiras execuções de I dão resultado errado
(--cold-start=5,I).

    python3 test_runs/analyze_m6.py [PAPER_DIR] [--list] [--export]

Critérios:
    detectou   o programa encontrou a divergência
    minimizou  "Minimized instruction set" tem 1 instrução e ela é I
    rotulou    "Could be cold start problem"
"""

from paper_analysis import main_for, same_instruction

EXPERIMENT = "M6"
TITLE = "Cold start"
CRITERIA = [
    "detectou:  o programa encontrou a divergência",
    "minimizou: o conjunto minimizado é só a instrução injetada I",
    "rotulou:   Could be cold start problem",
]


def minimized(fault, out):
    return len(out.minimized) == 1 and same_instruction(out.minimized_names[0], fault)


def labeled(fault, out):
    return out.cold_start


if __name__ == "__main__":
    main_for(EXPERIMENT, TITLE, CRITERIA, minimized, labeled)
