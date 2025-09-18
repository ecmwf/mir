# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


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
    assert j.json == '{"key":"{}"}'

    j.set("key", dict())
    assert j.json == Job(key=dict()).json
    assert j.json == '{"key":"{}"}'

    j.set("key", dict(nested={}))
    assert j.json == Job(key=dict(nested={})).json
    assert j.json == '{"key":"{nested: {}}"}'


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
