#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path

TARGET_STRING = "Failed on test"


def scan_folder(folder_path: Path, recursive: bool = False):
    if not folder_path.exists() or not folder_path.is_dir():
        print(
            f"Error: Path '{folder_path}' does not exist or is not a directory.",
            file=sys.stderr,
        )
        sys.exit(1)

    # Find .out files (glob for shallow, rglob if recursive search is desired)
    file_pattern = "**/*.out" if recursive else "*.out"
    out_files = list(folder_path.glob(file_pattern))

    if not out_files:
        print(f"No '.out' files found in '{folder_path}'.")
        return

    failed_files = []
    clean_file_count = 0

    for file_path in out_files:
        try:
            # Using errors='replace' to prevent crashing on binary/non-UTF-8 characters
            with open(
                file_path, "r", encoding="utf-8", errors="replace"
            ) as f:
                content = f.read()
                if TARGET_STRING in content:
                    failed_files.append(file_path)
                else:
                    clean_file_count += 1
        except Exception as e:
            print(f"Warning: Could not read file '{file_path}': {e}")

    # Output Results
    print("=" * 50)
    print(f"Scan Summary for: {folder_path.resolve()}")
    print("=" * 50)
    print(f"Total .out files checked : {len(out_files)}")
    print(f"Files without failures   : {clean_file_count}")
    print(f"Files with failures      : {len(failed_files)}")
    print("=" * 50)

    if failed_files:
        print(f"\nFiles containing '{TARGET_STRING}':")
        for fail in failed_files:
            print(f"  [FAIL] {fail.name}  ({fail.resolve()})")
    else:
        print(f"\nAll .out files passed without '{TARGET_STRING}'!")


def main():
    parser = argparse.ArgumentParser(
        description="Scan .out files in a directory for failure strings."
    )
    parser.add_argument(
        "folder", type=Path, help="Target directory containing .out files"
    )
    parser.add_argument(
        "-r",
        "--recursive",
        action="store_true",
        help="Search subdirectories recursively",
    )

    args = parser.parse_args()
    scan_folder(args.folder, recursive=args.recursive)


if __name__ == "__main__":
    main()