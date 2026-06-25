# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import pytest

from mir import Grid


def test_grid_box_areas():
    grid = Grid(grid=[3, 3])
    areas = grid.grid_box_areas()

    assert len(areas) == len(grid)
    assert all(a > 0.0 for a in areas)
    assert sum(areas) == pytest.approx(510.0e12, rel=1e-2)  # Earth ~= 510,000,000 km^2


if __name__ == "__main__":
    pytest.main([__file__])
