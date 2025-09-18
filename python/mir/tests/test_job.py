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


def test_bool():
    j = Job()
    j.set("key", True)
    assert Job(key=True).json == j.json == '{"key":1}'

    j.set("key", False)
    assert Job(key=False).json == j.json == '{"key":0}'


def test_dict():
    j = Job()
    j.set("key", {})
    assert Job(key={}).json == j.json == '{"key":"{}"}'

    j.set("key", dict())
    assert Job(key=dict()).json == j.json == '{"key":"{}"}'

    j.set("key", dict(nested={}))
    assert Job(key=dict(nested={})).json == j.json == '{"key":"{nested: {}}"}'


def test_str():
    j = Job()
    j.set("key", "")
    assert Job(key="").json == j.json == '{"key":""}'

    j.set("key", "value")
    assert Job(key="value").json == j.json == '{"key":"value"}'


def test_int():
    j = Job()
    j.set("key", 1)
    assert Job(key=1).json == j.json == '{"key":1}'

    j.set("key", -1)
    assert Job(key=-1).json == j.json == '{"key":-1}'


def test_float():
    j = Job()
    j.set("key", 1.1)
    assert Job(key=1.1).json == j.json == '{"key":1.1}'

    j.set("key", 01.1000)
    assert Job(key=01.1000).json == j.json == '{"key":1.1}'

    j.set("key", -1e9)
    assert Job(key=-1e9).json == j.json == '{"key":-1e+09}'


if __name__ == "__main__":
    pytest.main([__file__])
