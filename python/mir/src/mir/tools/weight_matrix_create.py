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
from typing import Dict
from typing import List
from typing import Optional

import yaml
from mir.griddef import griddef_from_latlon
from mir.weight_matrix import WeightMatrix


def _yaml_from_dict(d):
    return yaml.dump(d, default_flow_style=True).strip()


def make_matrix(
    in_grid: Optional[Dict] = None,
    in_lat: Optional[List] = None,
    in_lon: Optional[List] = None,
    out_grid: Optional[Dict] = None,
    out_lat: Optional[List] = None,
    out_lon: Optional[List] = None,
    output=None,
    **kwargs,
):
    import mir

    ext = Path(output).suffix if output is not None else None
    if output is not None and ext not in (".mat", ".npz"):
        raise ValueError("make_matrix: output must have extension .mat or .npz")

    job = mir.Job()

    if in_grid is not None and in_lat is None and in_lon is None:
        input = mir.GridSpecInput(_yaml_from_dict(in_grid))
    elif in_grid is None and in_lat is not None and in_lon is not None:
        input = mir.GriddefInput(griddef_from_latlon(in_lat, in_lon, mir.cache()))
    else:
        raise ValueError("make_matrix: input grid or lats/lons must be provided.")

    if out_grid is not None and out_lat is None and out_lon is None:
        job.set("grid", _yaml_from_dict(out_grid))
    elif out_grid is None and out_lat is not None and out_lon is not None:
        job.set("griddef", griddef_from_latlon(out_lat, out_lon, mir.cache()))
    else:
        raise ValueError("make_matrix: output grid or lats/lons must be provided.")

    mat = Path(output)
    if ext == ".mat":
        job.set("interpolation-matrix", str(mat))
    elif ext == ".npz":
        mat = mat.with_name(mat.name + ".mat")
        job.set("interpolation-matrix", str(mat))

    for key, val in kwargs.items():
        job.set(key, val)

    try:
        job.execute(input, mir.EmptyOutput())
    except Exception as e:
        raise RuntimeError(f"make_matrix: error: {e}.") from e

    if ext and not mat.exists():
        raise FileNotFoundError(f"make_matrix: matrix file '{mat}' not found.")

    wm = WeightMatrix.from_mat(mat)

    if ext == ".npz":
        wm.to_npz(output)
        mat.unlink()
    elif ext == ".mat":
        pass  # already written by mir
    else:
        return wm


def main():
    p = argparse.ArgumentParser(description="Create interpolation weight matrix via mir")
    p.add_argument("--in-grid",  type=str, help="Input grid spec (YAML dict)")
    p.add_argument("--out-grid", type=str, help="Output grid spec (YAML dict)")
    p.add_argument("--output",   type=Path, required=True, help="Output file (.mat or .npz)")
    args = p.parse_args()

    import yaml as _yaml

    in_grid  = _yaml.safe_load(args.in_grid)  if args.in_grid  else None
    out_grid = _yaml.safe_load(args.out_grid) if args.out_grid else None

    make_matrix(in_grid=in_grid, out_grid=out_grid, output=args.output)
    print(f"Created {args.output}")


if __name__ == "__main__":
    main()
