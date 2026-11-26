# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Convolution kernels (k-nearest distance weightings), as filters: the output grid is the input grid.

Artificial fields on a few grid types (regular lat-lon, reduced Gaussian, HEALPix), with known results:
- radial smoothing (top-hat, hann, epanechnikov, gaussian): constants are kept, noise is reduced
- directional smoothing: only variations along the kernel direction are smoothed
- signed kernels: derivative (per metre), laplacian (per square metre), difference-of-gaussians (edge detection)

Distances are in metres. The neighbour search ("nearest_method": "distance") should reach the kernel radius (compact
kernels) or a few radii (gaussian).
"""

import numpy as np
import pytest

import mir

R = 6371229.0  # Earth radius [m], as used by mir

# about 200 km between points
GRIDS = [dict(grid=[2, 2]), dict(grid="O48"), dict(grid="H32", order="nested")]


def convolve(grid, values, **spec):
    """Filter values on grid, by a k-nearest interpolation spec (to the same grid)"""
    output = mir.ArrayOutput()
    job = mir.Job(grid=grid.spec, interpolation=dict(type="k-nn", **spec), filter=True, caching=False)
    job.execute(mir.ArrayInput(values, grid.spec_str), output)

    assert mir.Grid(output.spec) == grid
    return output.values().ravel()


def latlons(grid):
    lat, lon = (np.radians(np.ravel(x)) for x in grid.to_latlons())
    return lat, lon


@pytest.fixture(params=GRIDS, ids=lambda g: str(g["grid"]))
def grid(request):
    return mir.Grid(request.param)


# --- radial smoothing


SMOOTHING = {
    "top-hat": dict(distance_weighting="top-hat", kernel_radius=500e3),
    "hann": dict(distance_weighting="hann", kernel_radius=500e3),
    "epanechnikov": dict(distance_weighting="epanechnikov", kernel_radius=500e3),
    "gaussian": dict(distance_weighting="gaussian", distance_weighting_gaussian_stddev=200e3),
}


@pytest.mark.parametrize("name", SMOOTHING)
def test_smoothing(grid, name):
    spec = dict(nearest_method="distance", distance=600e3, **SMOOTHING[name])

    # a constant field is kept
    constant = np.full(len(grid), 7.0)
    assert convolve(grid, constant, **spec) == pytest.approx(constant)

    # random noise is smoothed (lower variance), its mean kept
    noise = np.random.default_rng(0).standard_normal(len(grid))
    smooth = convolve(grid, noise, **spec)
    assert smooth.std() < 0.6 * noise.std()
    assert abs(smooth.mean() - noise.mean()) < 0.05


def test_top_hat_is_a_moving_average(grid):
    # top-hat = plain average of the points within the radius
    noise = np.random.default_rng(1).standard_normal(len(grid))
    a = convolve(grid, noise, nearest_method="distance", distance=500e3, distance_weighting="top-hat")
    b = convolve(grid, noise, nearest_method="distance", distance=500e3, distance_weighting="no")
    assert a == pytest.approx(b)


# --- directional smoothing


def test_directional(grid):
    # east-west waves (about 1300 km long at the equator), compared in the tropics
    lat, lon = latlons(grid)
    waves = np.cos(30 * lon)
    tropics = np.abs(lat) < np.radians(30)

    spec = dict(
        nearest_method="distance",
        distance=1000e3,
        distance_weighting="directional",
        kernel="gaussian",
        kernel_radius=400e3,  # along the kernel direction
        kernel_radius_across=50e3,
    )

    east_west = convolve(grid, waves, kernel_angle=0, **spec)
    north_south = convolve(grid, waves, kernel_angle=90, **spec)

    # the east-west kernel smooths the waves out, the north-south kernel keeps them
    assert np.abs(east_west[tropics]).max() < 0.2
    assert np.corrcoef(north_south[tropics], waves[tropics])[0, 1] > 0.9


# --- signed kernels


DIFFERENTIAL = dict(nearest_method="distance", distance=900e3, kernel="gaussian", kernel_radius=300e3)


def test_derivative(grid):
    lat, lon = latlons(grid)
    away_from_poles = np.abs(lat) < np.radians(60)

    # northward distance R lat: derivative 1 to the north, 0 to the east
    northward = R * lat
    north = convolve(grid, northward, distance_weighting="derivative", kernel_angle=90, **DIFFERENTIAL)
    east = convolve(grid, northward, distance_weighting="derivative", kernel_angle=0, **DIFFERENTIAL)
    assert north[away_from_poles] == pytest.approx(1.0, abs=0.02)
    assert east[away_from_poles] == pytest.approx(0.0, abs=0.02)

    # R cos(lat) sin(lon): derivative cos(lon) to the east
    field = R * np.cos(lat) * np.sin(lon)
    east = convolve(grid, field, distance_weighting="derivative", kernel_angle=0, **DIFFERENTIAL)
    assert east[away_from_poles] == pytest.approx(np.cos(lon[away_from_poles]), abs=0.02)


def test_laplacian(grid):
    lat, _ = latlons(grid)

    # constant: 0
    zero = convolve(grid, np.ones(len(grid)), distance_weighting="laplacian", **DIFFERENTIAL)
    assert zero == pytest.approx(0.0, abs=1e-15)

    # sin(lat), a spherical harmonic of degree 1: laplacian -2 sin(lat) / R^2
    field = np.sin(lat)
    laplacian = convolve(grid, field, distance_weighting="laplacian", **DIFFERENTIAL)
    assert laplacian * R**2 / -2.0 == pytest.approx(field, abs=0.05)


def test_difference_of_gaussians(grid):
    # band-pass: a constant vanishes, a single peak stands out at its point
    spec = dict(nearest_method="distance", distance=1500e3, distance_weighting="difference-of-gaussians")
    spec.update(kernel_radius=200e3, kernel_radius_outer=400e3)

    assert convolve(grid, np.full(len(grid), 5.0), **spec) == pytest.approx(0.0, abs=1e-12)

    peak = np.zeros(len(grid))
    peak[len(grid) // 3] = 1.0
    edges = convolve(grid, peak, **spec)
    assert edges.argmax() == len(grid) // 3
    assert edges.min() < 0.0  # negative ring around the peak


# --- the same kernels, for interpolation (k-nearest, to another grid)

TARGET = mir.Grid(dict(grid=[1.5, 1.5]))


def interpolate(grid, values, **spec):
    """Interpolate values from grid to TARGET, by a k-nearest interpolation spec"""
    output = mir.ArrayOutput()
    job = mir.Job(grid=TARGET.spec, interpolation=dict(type="k-nn", **spec), caching=False)
    job.execute(mir.ArrayInput(values, grid.spec_str), output)

    assert mir.Grid(output.spec) == TARGET
    return output.values().ravel()


INTERPOLATING = {
    **SMOOTHING,
    "directional": dict(
        distance_weighting="directional", kernel="hann", kernel_radius=500e3, kernel_radius_across=250e3, kernel_angle=45
    ),
}


@pytest.mark.parametrize("name", INTERPOLATING)
def test_interpolation(grid, name):
    spec = dict(nearest_method="distance", distance=600e3, **INTERPOLATING[name])
    lat, _ = latlons(grid)
    target_lat, _ = latlons(TARGET)

    # constants are kept, smooth fields approximated
    assert interpolate(grid, np.full(len(grid), 7.0), **spec) == pytest.approx(7.0)
    assert interpolate(grid, np.sin(lat), **spec) == pytest.approx(np.sin(target_lat), abs=0.05)


def test_interpolation_derivative_laplacian(grid):
    # derivative and laplacian at the output points, which are not input points
    lat, _ = latlons(grid)
    target_lat, _ = latlons(TARGET)
    away_from_poles = np.abs(target_lat) < np.radians(60)

    north = interpolate(grid, R * lat, distance_weighting="derivative", kernel_angle=90, **DIFFERENTIAL)
    assert north[away_from_poles] == pytest.approx(1.0, abs=0.02)

    laplacian = interpolate(grid, np.sin(lat), distance_weighting="laplacian", **DIFFERENTIAL)
    assert laplacian * R**2 / -2.0 == pytest.approx(np.sin(target_lat), abs=0.05)


if __name__ == "__main__":
    pytest.main([__file__])
