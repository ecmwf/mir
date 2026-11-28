#!/usr/bin/env python3
# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""
Plot mir-tile output: grid points (coloured by tile) and tile bounding boxes.

  mir-tile --grid=<gridspec> --tile=<tiling> [tiling options] --coordinates > tiles.json
  plot_tiles.py tiles.json [--no-points] [--no-bbox] [--no-labels] [--tiles 0 3] [--output tiles.png]

The plot is shown with the system viewer (open/xdg-open), or written to a file with --output.

Examples:

  # HEALPix nested: tiles of HEALPix pixels at nside=4 (192 tiles of 4096 points)
  mir-tile --grid=H256n --tile=healpix-nested --nside=4 --coordinates > h256n-healpix.json
  plot_tiles.py h256n-healpix.json

  # regular: 8 x 4 tiles on HEALPix nested
  mir-tile --grid=H256n --tile=regular --ni=8 --nj=4 --coordinates > h256n-regular.json
  plot_tiles.py h256n-regular.json

  # regular: 4 x 3 tiles within an area, written to file
  mir-tile --grid=O320 --tile=regular --area=70/-30/30/50 --ni=4 --nj=3 --coordinates > o320-area.json
  plot_tiles.py o320-area.json --output o320-area.png

  # equal-regions (leopardi): 32 equal area regions, including polar caps
  mir-tile --grid=O320 --tile=equal-regions --regions=32 --coordinates > o320-eq32.json
  plot_tiles.py o320-eq32.json --no-points

  # from stdin, only some tiles
  mir-tile --grid=O320 --tile=equal-regions --regions=32 --coordinates | plot_tiles.py --tiles 0 1 2 31

Requires numpy, matplotlib, cartopy (optional). Points require mir-tile --coordinates.
"""

import argparse
import json
import shutil
import subprocess
import sys
import tempfile

import matplotlib

matplotlib.use("Agg")  # file output only

import matplotlib.patheffects as pe  # noqa: E402
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from matplotlib.patches import Rectangle  # noqa: E402

try:
    import cartopy.crs as ccrs
except ImportError:
    ccrs = None


COLOURS = ["#2a78d6", "#eda100", "#1baf7a", "#e87ba4"]
INK = "#3d3d3a"
SURFACE = "#ffffff"
DPI = 300


def load(path):
    text = sys.stdin.read() if path == "-" else open(path).read()
    try:
        data, _ = json.JSONDecoder().raw_decode(text[text.index("{") :])
        return data
    except ValueError:
        sys.exit("plot_tiles: input is not mir-tile JSON:\n" + text[:500])


def expand(indices):
    """Indices from values and [first, last] ranges (or values only, mir-tile --expand-indices)."""
    if not any(isinstance(i, list) for i in indices):
        return np.asarray(indices, dtype=int)
    return np.concatenate([np.arange(i[0], i[1] + 1) if isinstance(i, list) else [i] for i in indices])


def lon_intervals(w, e):
    """Longitude intervals within [-180, 180] covering [w, e] (wrapping at the antimeridian)."""
    if e - w >= 360.0:
        return [(-180.0, 180.0)]
    w0 = (w + 180.0) % 360.0 - 180.0
    e0 = w0 + (e - w)
    return [(w0, e0)] if e0 <= 180.0 else [(w0, 180.0), (-180.0, e0 - 360.0)]


def touching(a, b):
    """Bounding boxes [n, w, s, e] touch or overlap (longitudes modulo 360)."""
    if a[2] > b[0] or b[2] > a[0]:
        return False
    return any(w1 <= e2 and w2 <= e1 for w1, e1 in lon_intervals(a[1], a[3]) for w2, e2 in lon_intervals(b[1], b[3]))


def colour_tiles(tiles):
    """Greedy colouring so touching tiles get different colours (least clash if not possible)."""
    colours = []
    for i, t in enumerate(tiles):
        used = [colours[k] for k in range(i) if touching(t["bounding-box"], tiles[k]["bounding-box"])]
        colours.append(min(range(len(COLOURS)), key=lambda c: (used.count(c), c)))
    return colours


def view(path):
    viewer = shutil.which("open" if sys.platform == "darwin" else "xdg-open")
    if viewer is None:
        print(f"plot_tiles: no viewer (open/xdg-open), plot written to {path}", file=sys.stderr)
        return
    subprocess.run([viewer, path], check=False)


def main():
    parser = argparse.ArgumentParser(description="Plot mir-tile output (points and bounding boxes per tile)")
    parser.add_argument("json", nargs="?", default="-", help="mir-tile JSON output (default: stdin)")
    parser.add_argument("--points", action=argparse.BooleanOptionalAction, default=True, help="plot points")
    parser.add_argument("--bbox", action=argparse.BooleanOptionalAction, default=True, help="plot bounding boxes")
    parser.add_argument("--labels", action=argparse.BooleanOptionalAction, default=True, help="label tile indices")
    parser.add_argument("--coastlines", action=argparse.BooleanOptionalAction, default=True, help="needs cartopy")
    parser.add_argument("--tiles", type=int, nargs="+", help="tile indices to plot (default: all)")
    parser.add_argument("--marker-size", type=float, help="point marker size (default: from number of points)")
    parser.add_argument("--output", help="output image file (default: temporary file, shown with open/xdg-open)")
    args = parser.parse_args()

    data = load(args.json)
    tiles = data["tiles"]
    colours = colour_tiles(tiles)

    selected = [(t, c) for t, c in zip(tiles, colours) if args.tiles is None or t["index"] in args.tiles]

    points = args.points and "latitudes" in data and "longitudes" in data
    if args.points and not points:
        print("plot_tiles: no coordinates (run mir-tile with --coordinates), not plotting points", file=sys.stderr)

    # longitudes are plotted in [-180, 180]
    fig = plt.figure(figsize=(12, 6.5), facecolor=SURFACE)
    if ccrs is not None and args.coastlines:
        ax = fig.add_subplot(projection=ccrs.PlateCarree())
        ax.coastlines(linewidth=0.5, color=INK)
        ax.set_global()
    else:
        if args.coastlines:
            print("plot_tiles: cartopy not available, not plotting coastlines", file=sys.stderr)
        ax = fig.add_subplot()
        ax.set_xlim(-180, 180)
        ax.set_ylim(-90, 90)
        ax.set_aspect("equal")
        ax.set_xlabel("longitude", color=INK)
        ax.set_ylabel("latitude", color=INK)
        ax.tick_params(colors=INK)
    ax.set_facecolor(SURFACE)

    if points:
        lats = np.asarray(data["latitudes"])
        lons = (np.asarray(data["longitudes"]) + 180.0) % 360.0 - 180.0
        size = args.marker_size or float(np.clip(2.0e4 / max(len(lats), 1), 0.05, 8.0))
        for t, c in selected:
            idx = expand(t["indices"])
            ax.scatter(lons[idx], lats[idx], s=size, color=COLOURS[c], linewidths=0, rasterized=True)

    for t, c in selected:
        n, w, s, e = t["bounding-box"]
        pieces = lon_intervals(w, e)  # two pieces if crossing the antimeridian
        if args.bbox:
            for w0, e0 in pieces:
                ax.add_patch(Rectangle((w0, s), e0 - w0, n - s, fill=False, edgecolor=COLOURS[c], linewidth=1.5))
        if args.labels:
            w0, e0 = max(pieces, key=lambda p: p[1] - p[0])  # label once, on the widest piece
            ax.text(
                (w0 + e0) / 2.0,
                (n + s) / 2.0,
                str(t["index"]),
                color=INK,
                fontsize=7,
                ha="center",
                va="center",
                path_effects=[pe.withStroke(linewidth=2, foreground=SURFACE)],
            )

    ax.set_title(
        f"{data['tile']}: {len(selected)} of {len(tiles)} tiles, {data['size']} points",
        color=INK,
    )

    path = args.output
    if path is None:
        with tempfile.NamedTemporaryFile(prefix="plot_tiles_", suffix=".png", delete=False) as f:
            path = f.name

    fig.savefig(path, dpi=DPI, bbox_inches="tight", facecolor=SURFACE)
    if args.output is None:
        view(path)


if __name__ == "__main__":
    main()
