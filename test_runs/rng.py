#!/usr/bin/env python3
import argparse
import numpy as np
import secrets # Add this at the top of your file with the other imports

def main():
    parser = argparse.ArgumentParser(description="Generate binary files with random distributions.")
    parser.add_argument('-n', '--count', type=int, required=True, help="Number of elements to generate")
    parser.add_argument('-d', '--dist', choices=['uniform', 'normal', 'lognormal', 'zeros', 'ones'], required=True, help="Distribution type")
    parser.add_argument('-t', '--dtype', choices=['float32', 'float64', 'int32', 'int64'], default='float32', help="Data type (default: float32)")
    parser.add_argument('-s', '--seed', type=int, default=None, help="Random seed for reproducibility")
    parser.add_argument('-o', '--output', type=str, help="Output binary file path")
    
    # Distribution specific parameters
    parser.add_argument('--low', type=float, default=0.0, help="Lower bound for uniform")
    parser.add_argument('--high', type=float, default=1.0, help="Upper bound for uniform")
    parser.add_argument('--mean', type=float, default=0.0, help="Mean for normal/lognormal")
    parser.add_argument('--std', type=float, default=1.0, help="Standard deviation for normal/lognormal")

    args = parser.parse_args()

    if args.seed is None:
        import sys
        args.seed = secrets.randbits(32)
        print(f"No seed provided. Using randomly generated seed: {args.seed}", file=sys.stderr)
    #else:
        #print(f"Using provided seed: {args.seed}", file=sys.stderr)

    rng = np.random.default_rng(args.seed)

    # Generate data based on the chosen distribution
    if args.dist == 'uniform':
        if 'int' in args.dtype:
            data = rng.integers(int(args.low), int(args.high), size=args.count)
        else:
            data = rng.uniform(args.low, args.high, size=args.count)
    elif args.dist == 'normal':
        data = rng.normal(args.mean, args.std, size=args.count)
    elif args.dist == 'lognormal':
        data = rng.lognormal(args.mean, args.std, size=args.count)
    elif args.dist == 'zeros':
        data = np.zeros(args.count)
    elif args.dist == 'ones':
        data = np.ones(args.count)

    # Cast to the requested data type
    try:
        data = data.astype(args.dtype)
    except ValueError as e:
        print(f"Error casting to {args.dtype}: {e}")
        sys.exit(1)

    # Save as raw binary
    if args.output:
        data.tofile(args.output)
        print(f"Successfully wrote {args.count} elements of type {args.dtype} ({data.nbytes} bytes) to {args.output}")
    else:
        import sys
        np.set_printoptions(threshold=sys.maxsize)
        print(data)

if __name__ == "__main__":
    main()