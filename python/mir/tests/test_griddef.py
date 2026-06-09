# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import pytest

pytest.importorskip("mir", exc_type=(ImportError, OSError))
from mir.griddef import griddef_from_latlon
from mir.griddef import write_latlon_to_griddef


def test_write_creates_file(tmp_path):
    path = tmp_path / "grid.griddef"
    write_latlon_to_griddef(path, [0.0, 1.0, 2.0], [10.0, 20.0, 30.0])
    assert path.exists()
    assert path.stat().st_size > 0


def test_write_empty_raises(tmp_path):
    with pytest.raises(ValueError):
        write_latlon_to_griddef(tmp_path / "g.griddef", [], [])


def test_write_length_mismatch_raises(tmp_path):
    with pytest.raises(ValueError):
        write_latlon_to_griddef(tmp_path / "g.griddef", [1.0], [2.0, 3.0])


def test_griddef_from_latlon_creates_file(tmp_path):
    path = griddef_from_latlon([0.0, 1.0], [10.0, 20.0], dir=tmp_path)
    assert path.endswith(".griddef")
    from pathlib import Path
    assert Path(path).exists()


def test_griddef_from_latlon(tmp_path):
    lats, lons = [0.0, 1.0], [10.0, 20.0]
    p1 = griddef_from_latlon(lats, lons, dir=tmp_path)
    p2 = griddef_from_latlon(lats, lons, dir=tmp_path)
    assert p1 == p2  # idempotent


def test_griddef_from_latlon_different_coords_differ(tmp_path):
    p1 = griddef_from_latlon([0.0], [0.0], dir=tmp_path)
    p2 = griddef_from_latlon([1.0], [1.0], dir=tmp_path)
    assert p1 != p2
