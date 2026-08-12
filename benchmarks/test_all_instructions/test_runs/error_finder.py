import argparse
import os
import re

def parse_file_errors(filepath, error_pattern):
    """Helper to parse a file and return its error count, or None if not found."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            for line in f:
                match = error_pattern.match(line)
                if match:
                    return int(match.group(1))
    except OSError as e:
        print(f"Could not read file {filepath}: {e}")
    return None

def analyze_error_files(directory, extension=".in"):
    zero_error_count = 0
    non_zero_files = []
    
    # Regex to match "Errors: <number>" (case-insensitive)
    error_pattern = re.compile(r'^\s*Errors:\s*(\d+)', re.IGNORECASE)

    if not os.path.isdir(directory):
        print(f"Error: Directory '{directory}' does not exist.")
        return

    # Find all matching files in the directory
    target_files = sorted([
        f for f in os.listdir(directory) 
        if f.endswith(extension) and os.path.isfile(os.path.join(directory, f))
    ])

    for filename in target_files:
        filepath = os.path.join(directory, filename)
        error_count = parse_file_errors(filepath, error_pattern)

        if error_count is not None:
            if error_count == 0:
                zero_error_count += 1
            else:
                non_zero_files.append((filename, error_count))

    # Display results
    print("--- Summary Report ---")
    print(f"Target Directory: {os.path.abspath(directory)}")
    #print(f"Extension Scanned: {extension}")
    print(f"Files scanned:       {len(target_files)}")
    print(f"Files with 0 errors: {zero_error_count}")
    print()
    
    if non_zero_files:
        print("\nFiles with errors detected:")
        for fname, err_val in non_zero_files:
            print(f"{fname} found {err_val} errors")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Analyze files for error counts in a directory.")
    parser.add_argument("folder", help="Path to the directory containing files")
    parser.add_argument("--ext", default=".out", help="File extension to analyze (default: .out)")
    
    args = parser.parse_args()
    
    analyze_error_files(args.folder, extension=args.ext)