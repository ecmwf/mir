# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


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
