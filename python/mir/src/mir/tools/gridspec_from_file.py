#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import argparse
from pathlib import Path

from mir._spec import to_yaml
from mir.gridspec import gridspec_from_file


def main():
    p = argparse.ArgumentParser(description="Print the grids (specs) of the fields in GRIB or netCDF files")
    p.add_argument("files", type=Path, nargs="+", help="GRIB or netCDF files")
    args = p.parse_args()

    for path in args.files:
        fields = {}
        for field, spec in gridspec_from_file(path).items():
            fields.setdefault(to_yaml(spec), []).append(field)

        print(f"{path}:")
        for spec, names in fields.items():
            print(f"- grid: {spec}")
            print(f"  fields: {to_yaml(names)}")


if __name__ == "__main__":
    main()
