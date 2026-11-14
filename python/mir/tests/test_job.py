# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import json
from pathlib import Path

import pytest

import mir
from mir import Job

TEST_DIR = Path(__file__).parent


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
    assert j.json == '{"key":true}'

    j.set("key", False)
    assert j.json == Job(key=False).json
    assert j.json == '{"key":false}'


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

    # the interpolation spec sets the interpolation (type) and its options (as the complete spec)
    j = Job()
    j.set("interpolation", dict(type="nearest-neighbour", nclosest=5))
    settings = json.loads(j.json)
    assert settings["interpolation"] == "nearest-neighbour"
    assert settings["nclosest"] == 5


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



def test_plan_json():
    path = str(TEST_DIR / "o96.grib2")
    job = Job(grid=[2, 2], accuracy=12, dryrun=True, dump_grid_plan_file="/dev/null")  # dryrun, dump: not affecting

    # one plan per message (consuming the input)
    (actions,) = map(json.loads, job.action_plan_json(mir.GribFileInput(path)))
    assert actions[0]["type"] == "interpolate.grid2regular-ll"
    assert actions[0]["grid"] == [2, 2]
    assert actions[-1] == dict(type="save", accuracy=12)

    # the input grid, and after each action
    (grids,) = map(json.loads, job.grid_plan_json(mir.GribFileInput(path)))
    assert len(grids) == len(actions) + 1
    assert grids[0] == dict(grid="O96")
    assert grids[-1] == grids[1]


if __name__ == "__main__":
    pytest.main([__file__])
