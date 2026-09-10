# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import hashlib
from pathlib import Path

from eckit.stream import Stream


def write_latlon_to_griddef(path, lats, lons):
    """Write a lat/lon point list to an eckit griddef binary file."""
    count = len(lats)
    if count != len(lons) or count == 0:
        raise ValueError("Latitudes and longitudes must be non-empty and have the same length.")

    with open(path, "wb") as f:
        s = Stream(f)
        s.write_unsigned_long(1)  # version
        s.write_unsigned_long(count)
        for lat, lon in zip(lats, lons):
            s.write_double(lat)
            s.write_double(lon)


def griddef_from_latlon(lats, lons, dir=None) -> str:
    """Return path to a griddef file for the given lat/lon arrays, creating it if needed."""
    coord = "1" + str(lats) + str(lons)
    path = Path(hashlib.md5(coord.encode()).hexdigest() + ".griddef")
    if dir:
        path = Path(dir) / path
    if not path.exists():
        write_latlon_to_griddef(path, lats, lons)
    return str(path)
