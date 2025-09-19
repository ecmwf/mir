# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


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
    assert (
        i.json
        == f'{{"type":"{type}","non-linear":["missing-if-heaviest-missing"],"interpolation-statistics":"{stat}"}}'
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


@pytest.mark.parametrize(
    "type", ["", "linear", "nearest-neighbour", "grid-box-average"]
)
def test_spec_as_str(type):
    interpolation = mir.Interpolation(type)
    assert not type or interpolation.type == type


@pytest.mark.parametrize(
    "type",
    (
        # "linear",
        "nearest-neighbour",
        # "grid-box-average"
    ),
)
def test_job_set_interpolationspec(type):
    interpolation = mir.Interpolation(interpolation=type)
    j = mir.Job(interpolation=interpolation.spec).json
    assert j == f'{{"interpolation":"{type}"}}'

    # nclosest (default 4) is only relevant for nearest-neighbour
    interpolation = mir.Interpolation(interpolation=type, nclosest=4)
    j = mir.Job(interpolation=interpolation.spec).json
    assert j == f'{{"interpolation":"{type}"}}'

    interpolation = mir.Interpolation(interpolation=type, nclosest=5)
    j = mir.Job(interpolation=interpolation.spec).json
    if type == "nearest-neighbour":
        assert (
            j
            == '{"distance-weighting":"inverse-distance-weighting-squared","interpolation":"nearest-neighbour","nclosest":5,"nearest-method":"nearest-neighbour-with-lowest-index","non-linear":"missing-if-heaviest-missing"}'
        )
    else:
        assert j == f'{{"interpolation":"{type}"}}'


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
    j = mir.Job(interpolation=interpolation.spec)
    assert (
        j.json
        == f'{{"interpolation":"{type}","interpolation-statistics":"{stat}","non-linear":"missing-if-heaviest-missing"}}'
    )


if __name__ == "__main__":
    pytest.main([__file__])
