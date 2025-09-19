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

# def dict_to_str(d) -> str:
#     from json import dumps
#     return dumps(d)


# def str_to_dict(s) -> dict:
#     from yaml import safe_load
#     return safe_load(s)


def test_interpolationspec_linear():
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


def test_interpolationspec_bilinear():
    type = "bilinear"

    interpol = mir.Interpolation(type)
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(interpolation=type, nclosest=4)
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(dict(interpolation=type))
    assert interpol.type == interpol.spec == type


def test_interpolationspec_nearest_neighbour():
    type = "nearest-neighbour"

    for alias in [type, "nearest-neighbor", "nn"]:
        interpol = mir.Interpolation(alias)
        assert interpol.type == interpol.spec == type

        interpol = mir.Interpolation(dict(interpolation=alias))
        assert interpol.type == interpol.spec == type


def test_interpolationspec_grid_box_average():
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
def test_interpolationspec_statistics(type, stat):
    interpol = mir.Interpolation(interpolation=type, interpolation_statistics=stat)
    assert interpol.type == type
    assert (
        interpol.spec_str
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
    assert j.json == f'{{"interpolation":"{{interpolation-statistics: {stat}, non-linear: [missing-if-heaviest-missing], type: {type}}}"}}'


if __name__ == "__main__":
    pytest.main([__file__])
