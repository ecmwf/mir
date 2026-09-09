# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import pytest

from mir import Job


def test_key():
    v = "value"
    j = Job(key=v)
    assert j.json == Job(key=v).json
    assert j.json == f'{{"key":"{v}"}}'

    v = "a_very-long_value"
    j = Job()
    j.set("a_very-long_key", v)
    assert j.json == Job(a_very_long_key=v).json
    assert j.json == f'{{"a-very-long-key":"{v}"}}'


def test_value_bool():
    j = Job(key=True)
    assert j.json == Job(key=True).json
    assert j.json == '{"key":1}'

    j.set("key", False)
    assert j.json == Job(key=False).json
    assert j.json == '{"key":0}'


def test_value_dict():
    j = Job(key={})
    assert j.json == Job(key={}).json
    assert j.json == "{}"

    j.set("key", dict())
    assert j.json == Job(key=dict()).json
    assert j.json == "{}"

    k = Job()
    k.set("ab", "cd").set("ef", dict(type="gh"))
    assert k.json == Job(ab=dict(type="cd", nested=dict(again=dict(ef="gh")))).json
    assert k.json == '{"ab":"cd","ef":"gh"}'


def test_value_dict_set_grid_and_interpolation_yaml():
    j = Job()
    j.set("grid", dict(grid="H16", order="nested"))
    assert j.json == '{"grid":"{grid: H16, order: nested}"}'

    j = Job()
    j.set("interpolation", dict(type="nearest-neighbour", nclosest=5))
    assert j.json == '{"interpolation":"{nclosest: 5, type: nearest-neighbour}"}'


def test_value_str():
    j = Job(key="")
    assert j.json == Job(key="").json
    assert j.json == '{"key":""}'

    j.set("key", "value")
    assert j.json == Job(key="value").json
    assert j.json == '{"key":"value"}'


def test_value_int():
    j = Job(key=1)
    assert j.json == Job(key=1).json
    assert j.json == '{"key":1}'

    j.set("key", -1)
    assert j.json == Job(key=-1).json
    assert j.json == '{"key":-1}'


def test_value_float():
    j = Job(key=1.1)
    assert j.json == Job(key=1.1).json
    assert j.json == '{"key":1.1}'

    j.set("key", 01.1000)
    assert j.json == Job(key=01.1000).json
    assert j.json == '{"key":1.1}'

    j.set("key", -1e9)
    assert j.json == Job(key=-1e9).json
    assert j.json == '{"key":-1e+09}'


if __name__ == "__main__":
    pytest.main([__file__])
