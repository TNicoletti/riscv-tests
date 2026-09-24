#!/usr/bin/env python3
from datetime import datetime
import re
import sys
from collections import defaultdict
from pathlib import Path


PATTERNS = [
    (r"Minimized instruction set with 1 instructions:", 1),
    (r"Minimized instruction set with 2 instructions:", 2),
    (r"Minimized instruction set with 3 instructions:", 3),
    (r"Minimized instruction set with 4 instructions:", 4),
]

COLD_START_PATTERN = r"Could be cold start problem"
RAW_PATTERN = r"Possible Read after Write data hazard detected"
WAW_PATTERN = r"Possible Write after Write data hazard detected"

TAIL_LINES = 200


def read_last_lines(file_path: Path, n: int = TAIL_LINES) -> str:
    """Lê apenas as últimas n linhas do arquivo."""
    try:
        with file_path.open("r", encoding="utf-8", errors="ignore") as f:
            lines = f.readlines()

        return "".join(lines[-n:])

    except Exception as e:
        print(
            f"[Aviso] Erro ao ler {file_path.name}: {e}",
            file=sys.stderr
        )
        return ""


def parse_error_sequence(content: str) -> str | int:
    """Identifica o tamanho da sequência mínima."""
    for pattern, seq_len in PATTERNS:
        if re.search(pattern, content):
            return seq_len

    return "Ausente"


def sort_key(p: Path):
    """Chave para ordenação natural por número no nome do arquivo."""
    numbers = re.findall(r"\d+", p.name)
    return int(numbers[0]) if numbers else p.name


def print_category(label, files, all_file_names, show_list, show_opposite):
    """Imprime quantidade e, opcionalmente, a lista de arquivos ou o oposto dela."""
    total_files = len(all_file_names)
    count = len(files)
    pct = (count / total_files * 100) if total_files else 0.0

    print(f"{label:<30}: {count:>4} ({pct:>5.1f}%)")

    if show_list:
        if files:
            print("  " + " ".join(files))
        else:
            print("  (nenhum)")
    elif show_opposite:
        opposite_files = [f for f in all_file_names if f not in files]
        if opposite_files:
            print("  " + " ".join(opposite_files))
        else:
            print("  (nenhum)")


def main():
    if len(sys.argv) < 2:
        print(f"Uso: python {sys.argv[0]} <caminho_da_pasta> [--list | --opositelist]")
        sys.exit(1)

    target_dir = Path(sys.argv[1])
    show_list = "--list" in sys.argv[2:]
    show_opposite = "--opositelist" in sys.argv[2:]

    if not target_dir.is_dir():
        print(
            f"Erro: '{target_dir}' não é um diretório válido.",
            file=sys.stderr
        )
        sys.exit(1)

    out_files = sorted(target_dir.glob("*.out"), key=sort_key)

    if not out_files:
        print(f"Nenhum arquivo *.out encontrado em '{target_dir}'.")
        return

    first_file = out_files[0]
    mtime = first_file.stat().st_mtime
    first_file_date = datetime.fromtimestamp(mtime).strftime("%Y-%m-%d %H:%M:%S")

    all_file_names = [f.name for f in out_files]

    # ============================================================
    # Processamento
    # ============================================================

    categorized = defaultdict(list)

    cold_start_files = []
    raw_files = []
    waw_files = []

    for file_path in out_files:
        # Os resultados relevantes ficam no final do arquivo.
        content = read_last_lines(file_path)

        # Minimização
        result = parse_error_sequence(content)
        categorized[result].append(file_path.name)

        # Cold start
        if re.search(COLD_START_PATTERN, content):
            cold_start_files.append(file_path.name)

        # RAW
        if re.search(RAW_PATTERN, content):
            raw_files.append(file_path.name)

        # WAW
        if re.search(WAW_PATTERN, content):
            waw_files.append(file_path.name)

    total_files = len(out_files)
    categories_order = [1, 2, 3, 4, "Ausente"]

    # ============================================================
    # Resumo da minimização
    # ============================================================

    print("=" * 60)
    print(f" RESUMO GERAL ({total_files} arquivos processados)")
    print(f" Data do 1º arquivo ({first_file.name}): {first_file_date}")
    print("=" * 60)

    for cat in categories_order:
        files = categorized.get(cat, [])
        label = (
            f"Resposta {cat}"
            if isinstance(cat, int)
            else "Ausência / Não Identificado"
        )
        print_category(label, files, all_file_names, show_list, show_opposite)

    # ============================================================
    # Cold Start / RAW / WAW
    # ============================================================

    print("\n" + "=" * 60)
    print(" OUTROS RESULTADOS")
    print("=" * 60)

    print_category(
        "Possível Cold Start",
        cold_start_files,
        all_file_names,
        show_list,
        show_opposite
    )

    print_category(
        "Possível RAW",
        raw_files,
        all_file_names,
        show_list,
        show_opposite
    )

    print_category(
        "Possível WAW",
        waw_files,
        all_file_names,
        show_list,
        show_opposite
    )


if __name__ == "__main__":
    main()