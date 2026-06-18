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
