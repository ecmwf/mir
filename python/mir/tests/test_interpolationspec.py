# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import json

import pytest

import mir


def test_linear():
    # None: assumes the interpolation default is linear
    type = "linear"

    interpol = mir.Interpolation()
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(nclosest=4)
    assert interpol.type == interpol.spec == type

    for alias in [type, "", None]:
        interpol = mir.Interpolation(alias)
        assert interpol.type == interpol.spec == type

        interpol = mir.Interpolation(dict(interpolation=alias))
        assert interpol.type == interpol.spec == type


def test_bilinear():
    type = "bilinear"

    interpol = mir.Interpolation(type)
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(interpolation=type, nclosest=4)
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(dict(interpolation=type))
    assert interpol.type == interpol.spec == type


def test_nearest_neighbour():
    type = "nearest-neighbour"

    for alias in [type, "nearest-neighbor", "nn"]:
        interpol = mir.Interpolation(alias)
        assert interpol.type == interpol.spec == type

        interpol = mir.Interpolation(dict(interpolation=alias))
        assert interpol.type == interpol.spec == type


def test_grid_box_average():
    type = "grid-box-average"

    interpol = mir.Interpolation(type)
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(dict(interpolation=type))
    assert interpol.type == interpol.spec == type


@pytest.mark.parametrize(
    "type, stat",
    [
        ("grid-box-statistics", "maximum"),
        ("grid-box-statistics", "minimum"),
        ("voronoi-statistics", "maximum"),
        ("voronoi-statistics", "minimum"),
    ],
)
def test_statistics(type, stat):
    i = mir.Interpolation(interpolation=type, interpolation_statistics=stat)
    assert i.type == type

    # the type alone, with its defaults (maximum)
    assert i.json == (
        type
        if stat == "maximum"
        else f'{{"type":"{type}","non-linear":["missing-if-heaviest-missing"],"interpolation-statistics":"{stat}"}}'
    )


SPECS = [
    (dict(), "linear"),
    (dict(interpolation="linear"), "linear"),
    (dict(interpolation="nn"), "nearest-neighbour"),
    (dict(interpolation="nearest-neighbour"), "nearest-neighbour"),
    (dict(interpolation="grid-box-average"), "grid-box-average"),
]


@pytest.mark.parametrize("interpolation, spec", SPECS)
def test_simple_specs(interpolation, spec):
    interpolation = mir.Interpolation(interpolation)
    assert interpolation.spec == spec


@pytest.mark.parametrize("type", ["", "linear", "nearest-neighbour", "grid-box-average"])
def test_spec_as_str(type):
    interpolation = mir.Interpolation(type)
    assert not type or interpolation.type == type


def job_interpolation(job):
    """The interpolation set on the job (type and options, as settings)"""
    return mir.Interpolation(json.loads(job.json))


@pytest.mark.parametrize("type", ["nearest-neighbour"])
def test_job_set_interpolationspec(type):
    for interpolation in (
        mir.Interpolation(interpolation=type),
        mir.Interpolation(interpolation=type, nclosest=4),  # nclosest (default 4)
        mir.Interpolation(interpolation=type, nclosest=5),
    ):
        # as spec, or as object
        for value in (interpolation.spec, interpolation):
            j = mir.Job()
            j.set("interpolation", value)
            assert job_interpolation(j) == interpolation


@pytest.mark.parametrize(
    "type, stat",
    [
        ("grid-box-statistics", "maximum"),
        ("grid-box-statistics", "minimum"),
        ("voronoi-statistics", "maximum"),
        ("voronoi-statistics", "minimum"),
    ],
)
def test_job_set_interpolationspec_statistics(type, stat):
    interpolation = mir.Interpolation(interpolation=type, interpolation_statistics=stat)
    j = mir.Job()
    j.set("interpolation", interpolation.spec)
    assert job_interpolation(j) == interpolation


if __name__ == "__main__":
    pytest.main([__file__])
