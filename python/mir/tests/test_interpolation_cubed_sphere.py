# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import numpy as np
import pytest

import mir

O96 = dict(grid="O96")


def field(spec):
    # smooth, so linear interpolation errors are small
    lat, lon = (np.radians(np.asarray(c)) for c in mir.Grid(spec).to_latlons())
    return np.cos(lat) ** 2 * np.cos(2 * lon) + np.sin(lat)


def check_interpolation(a, b):
    output = mir.ArrayOutput()
    mir.Job(grid=b, interpolation="linear").execute(mir.ArrayInput(field(a), mir.Grid(a).spec_str), output)

    # the output is described by its spec (e.g. by name), not by its points
    grid = mir.Grid(b)
    assert output.spec == grid.spec
    assert output.shape == grid.shape

    values = output.values()
    assert values.shape == grid.shape
    assert np.abs(values.reshape(-1) - field(b)).max() < 2e-3


CUBED_SPHERES = [dict(grid="CS-LFR-56"), dict(grid="CS-LFR-56-NODE")]


@pytest.mark.parametrize("cubed_sphere", CUBED_SPHERES)
def test_interpolation_to_cubed_sphere(cubed_sphere):
    check_interpolation(O96, cubed_sphere)


@pytest.mark.parametrize("cubed_sphere", CUBED_SPHERES)
def test_interpolation_from_cubed_sphere(cubed_sphere):
    check_interpolation(cubed_sphere, O96)
