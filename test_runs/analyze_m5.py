#!/usr/bin/env python3
"""
Analisa o M5 (sequence): quando a sequência I1 -> I2 -> I3 (-> I4) roda consecutivamente,
a última vira NOP (--three/four-instruction-err).

    python3 test_runs/analyze_m5.py [PAPER_DIR] [--list] [--export]

O relatório sai separado em triplas (three*) e quádruplas (four*); o CSV tem as duas.

Critérios:
    detectou   o programa encontrou a divergência
    minimizou  "Minimized instruction set" é exatamente a sequência injetada, na ordem
    rotulou    "Error is compulsory" (determinística) e a heurística 2 diz que o problema
               envolve várias instruções (não uma instrução única)
"""

from paper_analysis import main_for, same_instruction

EXPERIMENT = "M5"
TITLE = "Sequence"
CRITERIA = [
    "detectou:  o programa encontrou a divergência",
    "minimizou: o conjunto minimizado é a sequência injetada, na mesma ordem",
    "rotulou:   determinística e dependente de várias instruções (heurística 2)",
]


GROUPS = [("three", "Sequence triplet"), ("four", "Sequence quadruplet")]


def groups(results):
    """Separa as falhas em triplas (three*) e quádruplas (four*)."""
    return [(group_title, [r for r in results if r.fault.startswith(prefix)])
            for prefix, group_title in GROUPS]


def sequence(fault):
    # three00_vmv_v_v-vor_vv-vand_vx -> [vmv_v_v, vor_vv, vand_vx]
    return fault.split("_", 1)[1].split("-")


def minimized(fault, out):
    seq = sequence(fault)
    names = out.minimized_names
    return len(names) == len(seq) and all(same_instruction(a, b) for a, b in zip(names, seq))


def labeled(fault, out):
    return out.compulsory is True and out.multiple_instructions and not out.single_instruction


if __name__ == "__main__":
    main_for(EXPERIMENT, TITLE, CRITERIA, minimized, labeled, groups_fn=groups)
