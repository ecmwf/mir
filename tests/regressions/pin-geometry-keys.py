#!/usr/bin/env python3
# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Pin the GRIB geometry keys of regression test outputs as '# grib_get' assertions.

The geometry of an output can be encoded by GRIB keys (legacy) or by gridSpec; pinning the keys that describe it, from
the legacy outputs, makes any difference of the other modes visible as a readable assertion (key, expected, got).
Keys already asserted are kept as they are; keys not in a message aren't pinned.

Usage (after the legacy tests ran, in the build tree):
    pin-geometry-keys.py --grib-get=<grib_get> --work-dir=<build>/mir/tests/regressions <test>...
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

LL = [
    "Ni",
    "Nj",
    "latitudeOfFirstGridPointInDegrees",
    "longitudeOfFirstGridPointInDegrees",
    "latitudeOfLastGridPointInDegrees",
    "longitudeOfLastGridPointInDegrees",
    "iDirectionIncrementInDegrees",
    "jDirectionIncrementInDegrees",
    "iScansNegatively",
    "jScansPositively",
]

ROTATION = [
    "latitudeOfSouthernPoleInDegrees",
    "longitudeOfSouthernPoleInDegrees",
    "angleOfRotationInDegrees",
    "uvRelativeToGrid",
]

GG = ["N"] + LL

REDUCED = [
    "N",
    "Nj",
    "isOctahedral",
    "latitudeOfFirstGridPointInDegrees",
    "longitudeOfFirstGridPointInDegrees",
    "latitudeOfLastGridPointInDegrees",
    "longitudeOfLastGridPointInDegrees",
    "iScansNegatively",
    "jScansPositively",
]

XY = [
    "Nx",
    "Ny",
    "latitudeOfFirstGridPointInDegrees",
    "longitudeOfFirstGridPointInDegrees",
    "DxInMetres",
    "DyInMetres",
    "iScansNegatively",
    "jScansPositively",
    "uvRelativeToGrid",
]

KEYS = {
    "regular_ll": LL,
    "rotated_ll": LL + ROTATION,
    "regular_gg": GG,
    "rotated_gg": GG + ROTATION,
    "reduced_gg": REDUCED,
    "reduced_rotated_gg": REDUCED + ROTATION,
    "reduced_ll": ["Nj"] + LL[2:6] + ["jDirectionIncrementInDegrees"],
    "healpix": ["Nside", "orderingConvention", "longitudeOfFirstGridPointInDegrees"],
    "lambert": XY + ["LaDInDegrees", "LoVInDegrees", "Latin1InDegrees", "Latin2InDegrees"],
    "lambert_azimuthal_equal_area": XY + ["standardParallelInDegrees", "centralLongitudeInDegrees"],
    "polar_stereographic": XY + ["LaDInDegrees", "orientationOfTheGridInDegrees", "southPoleOnProjectionPlane"],
    "unstructured_grid": ["numberOfGridUsed", "numberOfGridInReference", "uuidOfHGrid"],
    "sh": ["J", "K", "M"],
}

HEADER = re.compile(r"^# (input|post-process|action-plan|grid-plan|grib_get)( (.*))?$")
COUNT = re.compile(r"^(count=[1-9][0-9]*)( |$)")


def grib_get(tool, path, count, key):
    p = subprocess.run([tool, "-w", count, "-p", key, str(path)], capture_output=True, text=True)
    value = p.stdout.strip()
    return None if p.returncode != 0 or value in ("", "not_found") else value


def geometry_keys(tool, path, count):
    grid_type = grib_get(tool, path, count, "gridType")
    if grid_type is None:
        return []

    keys = ["gridType", "numberOfDataPoints"] + KEYS.get(grid_type, [])
    if grib_get(tool, path, count, "edition") == "2":
        keys.append("shapeOfTheEarth")
    return keys


def pin(test, tool, work_dir):
    out = work_dir / f"data.out.{test.name}.legacy"
    if not out.exists() or out.read_bytes()[:4] != b"GRIB":
        return False

    lines = test.read_text().splitlines()

    # grib_get blocks: message (count=N) -> (index of the last line of the block, keys asserted)
    blocks = {}
    section = message = None
    for i, line in enumerate(lines):
        m = HEADER.match(line)
        if m:
            section = m.group(1)
            message = "count=1"
            c = COUNT.match(m.group(3) or "")
            if c:
                message = c.group(1)
            if section == "grib_get":
                blocks.setdefault(message, [i, set()])
            continue
        if section == "grib_get" and line.strip() and not line.startswith("#"):
            blocks[message][0] = i
            blocks[message][1].add(line.split("=", 1)[0].strip())

    if "count=1" not in blocks:
        lines.append("# grib_get")
        blocks["count=1"] = [len(lines) - 1, set()]

    # insert from the bottom, so the indices of the blocks above stay valid
    changed = False
    for message, (last, asserted) in sorted(blocks.items(), key=lambda b: b[1][0], reverse=True):
        new = []
        for key in geometry_keys(tool, out, message):
            if key not in asserted:
                value = grib_get(tool, out, message, key)
                if value is not None:
                    new.append(f"{key}={value}")
        if new:
            lines[last + 1 : last + 1] = new
            changed = True

    if changed:
        test.write_text("\n".join(lines) + "\n")
    return changed


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--grib-get", required=True)
    parser.add_argument("--work-dir", required=True, type=Path)
    parser.add_argument("tests", nargs="+", type=Path)
    args = parser.parse_args()

    for test in args.tests:
        if pin(test, args.grib_get, args.work_dir):
            print(f"pinned: {test.name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
