#!/usr/bin/env python3
"""
Analisa o M1 (illegal instruction): a instrução I gera trap de instrução ilegal.

    python3 test_runs/analyze_m1.py [PAPER_DIR] [--list] [--export]

Critérios:
    detectou   o programa encontrou a divergência
    minimizou  "Minimized instruction set" tem 1 instrução e ela é I
    rotulou    "Error is compulsory" (determinística) e a heurística 2 aponta uma única
               instrução, que é I ("Probably problematic instruction: I")
"""

from paper_analysis import main_for, same_instruction

EXPERIMENT = "M1"
TITLE = "Illegal instruction"
CRITERIA = [
    "detectou:  o programa encontrou a divergência",
    "minimizou: o conjunto minimizado é só a instrução injetada I",
    "rotulou:   determinística (Error is compulsory) e instrução única = I (heurística 2)",
]


def minimized(fault, out):
    return len(out.minimized) == 1 and same_instruction(out.minimized_names[0], fault)


def labeled(fault, out):
    return (out.compulsory is True and out.single_instruction
            and out.problematic is not None and same_instruction(out.problematic, fault))


if __name__ == "__main__":
    main_for(EXPERIMENT, TITLE, CRITERIA, minimized, labeled)
