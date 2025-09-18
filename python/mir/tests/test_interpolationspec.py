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

    for alias in [type, ""]:
        interpol = mir.Interpolation(alias)
        assert interpol.type == interpol.spec == type

        interpol = mir.Interpolation(dict(interpolation=alias))
        assert interpol.type == interpol.spec == type



def test_interpolationspec_bilinear():
    type = "bilinear"

    interpol = mir.Interpolation(type)
    assert interpol.type == interpol.spec == type

    interpol = mir.Interpolation(interpolation =type, nclosest=4)
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


GRIDSPECS = [
    (dict(), "linear"),
    (dict(interpolation="linear"), "linear"),
    (dict(interpolation="nn"), "nearest-neighbour"),
    (dict(interpolation="nearest-neighbour"), "nearest-neighbour"),
    (dict(interpolation="grid-box-average"), "grid-box-average"),
]


@pytest.mark.parametrize("interpolation, spec", GRIDSPECS)
def test_simple_specs(interpolation, spec):
    interpolation = mir.Interpolation(interpolation)
    assert interpolation.spec == spec


@pytest.mark.parametrize("type", ["", "linear", "nearest-neighbour", "grid-box-average"])
def test_spec_as_str(type):
    interpolation = mir.Interpolation(type)
    assert not type or interpolation.type == type


if __name__ == "__main__":
    pytest.main([__file__])
