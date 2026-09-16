#!/usr/bin/env python3
from datetime import datetime
import re
import sys
from collections import defaultdict
from pathlib import Path


PATTERNS = [
    (r"Minimum error sequence is a single instruction:", 1),
    (r"Minimum error sequence is a 2 instruction sequence:", 2),
    (r"Minimum error sequence is a 3 instruction sequence:", 3),
    (r"Minimum error sequence is the entire sequence:", 4),
]

COLD_START_PATTERN = r"Could be cold start problem"
RAW_PATTERN = r"Possible Read after Write data hazard detected"
WAW_PATTERN = r"Possible Write after Write data hazard detected"

TAIL_LINES = 100


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


def print_category(label, files, show_list):
    """Imprime quantidade e, opcionalmente, a lista de arquivos."""
    print(f"{label:<30}: {len(files):>4}")

    if show_list:
        if files:
            print("  " + " ".join(files))
        else:
            print("  (nenhum)")


def main():
    if len(sys.argv) < 2:
        print(f"Uso: python {sys.argv[0]} <caminho_da_pasta> [--list]")
        sys.exit(1)

    target_dir = Path(sys.argv[1])
    show_list = "--list" in sys.argv[2:]

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
        count = len(files)
        pct = (count / total_files * 100) if total_files else 0.0

        label = (
            f"Resposta {cat}"
            if isinstance(cat, int)
            else "Ausência / Não Identificado"
        )

        print(f"{label:<30}: {count:>4} ({pct:>5.1f}%)")

        if show_list:
            if files:
                print("  " + " ".join(files))
            else:
                print("  (nenhum)")

    # ============================================================
    # Cold Start / RAW / WAW
    # ============================================================

    print("\n" + "=" * 60)
    print(" OUTROS RESULTADOS")
    print("=" * 60)

    print_category(
        "Possível Cold Start",
        cold_start_files,
        show_list
    )

    print_category(
        "Possível RAW",
        raw_files,
        show_list
    )

    print_category(
        "Possível WAW",
        waw_files,
        show_list
    )


if __name__ == "__main__":
    main()