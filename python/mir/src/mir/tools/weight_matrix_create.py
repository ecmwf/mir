#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import argparse
from pathlib import Path

from mir.weight_matrix import WeightMatrix


def main():
    p = argparse.ArgumentParser(description="Create interpolation weight matrix")
    p.add_argument("--in-grid", type=str, required=True, help="Input grid spec")
    p.add_argument("--out-grid", type=str, required=True, help="Output grid spec")
    p.add_argument("--output", type=Path, required=True, help="Output file (.mat or .npz)")
    args, unknown_args = p.parse_known_args()

    try:
        extra = {}
        for token in unknown_args:
            key, sep, value = token.removeprefix("--").partition("=")
            if not token.startswith("--") or not sep or not key:
                raise ValueError(f"Invalid '{token}', expected --key=value.")
            extra[key.strip()] = value
    except ValueError as e:
        p.error(str(e))

    wm = WeightMatrix.from_regrid(in_grid=args.in_grid, out_grid=args.out_grid, **extra)
    wm.to_file(args.output)
    print(f"Created {args.output}")


if __name__ == "__main__":
    main()
