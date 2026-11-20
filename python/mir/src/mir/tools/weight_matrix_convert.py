#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import argparse
from pathlib import Path

from mir.weight_matrix import WeightMatrix


def main():
    p = argparse.ArgumentParser(description="Convert weight matrix between .mat and .npz, or from SCRIP weights (.nc)")
    p.add_argument("input", type=Path, help="Input file (.mat, .npz, or .nc with SCRIP weights)")
    p.add_argument("output", type=Path, help="Output file (.mat or .npz)")

    scrip = p.add_argument_group("SCRIP weights (.nc input)")
    scrip.add_argument("--nr", default="dst_grid_size", help="Dimension of the number of rows")
    scrip.add_argument("--nc", default="src_grid_size", help="Dimension of the number of columns")
    scrip.add_argument("--nnz", default="num_links", help="Dimension of the number of non-zeros (links)")
    scrip.add_argument("--ia", default="dst_address", help="Variable of the row indices (not compressed)")
    scrip.add_argument("--ja", default="src_address", help="Variable of the column indices")
    scrip.add_argument("--a", default="remap_matrix", help="Variable of the weights")
    scrip.add_argument("--base", type=int, default=1, help="Index base")
    args = p.parse_args()

    try:
        if args.input.suffix.lower() == ".nc":
            wm = WeightMatrix.from_scrip_weights(
                args.input, nr=args.nr, nc=args.nc, nnz=args.nnz, ia=args.ia, ja=args.ja, a=args.a, base=args.base
            )
        else:
            wm = WeightMatrix.from_file(args.input)
        written = wm.to_file(args.output)
    except Exception as e:
        p.error(str(e))

    print(f"{args.input} -> {written}  (shape={wm.shape}, nnz={wm.nnz})")


if __name__ == "__main__":
    main()
