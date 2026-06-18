#!/usr/bin/env python3

# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


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
