import sys
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

# Mapping user-friendly names to NumPy dtypes (handles endianness)
DTYPES = {
    'int8': '<i1',
    'uint8': '<u1',
    'int16': '<i2',
    'uint16': '<u2',
    'int32': '<i4',
    'uint32': '<u4',
    'int64': '<i8',
    'uint64': '<u8',
    'float16': '<f2',
    'float32': '<f4',
    'float64': '<f8',
}

def analyze_binary_file(filename, dtype_name):
    if dtype_name not in DTYPES:
        print(f"Error: Unsupported dtype '{dtype_name}'. Choose from: {list(DTYPES.keys())}")
        sys.exit(1)

    # Read raw binary directly into NumPy array
    dtype = np.dtype(DTYPES[dtype_name])
    try:
        data = np.fromfile(filename, dtype=dtype)
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

    print(f"Loaded {len(data):,} elements of type {dtype_name}")
    print(f"Min: {data.min()} | Max: {data.max()} | Mean: {data.mean():.4f} | Std Dev: {data.std():.4f}")

    # Plotting
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    # 1. Distribution Histogram + KDE
    sns.histplot(data, kde=True, ax=axes[0], bins=100, color='royalblue')
    axes[0].set_title(f"Distribution ({dtype_name})")
    axes[0].set_xlabel("Value")
    axes[0].set_ylabel("Count")

    # 2. Sequence plot (first 10,000 points to keep it light)
    sample_size = min(len(data), 10000)
    axes[1].plot(data[:sample_size], alpha=0.7, color='darkorange', linewidth=0.8)
    axes[1].set_title(f"Sequence (First {sample_size:,} elements)")
    axes[1].set_xlabel("Index / Offset")
    axes[1].set_ylabel("Value")

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 plot_data.py <filename> <dtype>")
        print("Example: python3 plot_data.py data.bin float32")
        sys.exit(1)

    filename = sys.argv[1]
    dtype_name = sys.argv[2]
    analyze_binary_file(filename, dtype_name)