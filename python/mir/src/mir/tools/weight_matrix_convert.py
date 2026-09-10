#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import argparse
from pathlib import Path

from mir.weight_matrix import WeightMatrix


def main():
    p = argparse.ArgumentParser(description="Convert weight matrix between .mat and .npz")
    p.add_argument("input", type=Path, help="Input file (.mat or .npz)")
    p.add_argument("output", type=Path, help="Output file (.mat or .npz)")
    args = p.parse_args()

    try:
        wm = WeightMatrix.from_file(args.input)
        written = wm.to_file(args.output)
    except Exception as e:
        p.error(str(e))

    print(f"{args.input} -> {written}  (shape={wm.shape}, nnz={wm.nnz})")


if __name__ == "__main__":
    main()
