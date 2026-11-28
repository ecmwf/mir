# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import mir


def test_cache():
    info = mir.cache()

    # a dict (sizes in bytes), as eckit.geo.cache
    assert isinstance(info, dict)
    assert {"memory", "disk"} <= set(info)

    # disk control: caching, throwing on a miss, and cache paths
    control = info["disk"]["control"]
    assert isinstance(control["caching"], bool)
    assert isinstance(control["throw_on_miss"], bool)
    assert set(control["paths"]) == {"readable", "writable"}

    for usage in info["memory"].values():
        assert set(usage) == {"footprint", "capacity"}
        assert all(isinstance(v, int) for v in usage["footprint"].values())

    for name, files in info["disk"].items():
        if name != "control":
            assert isinstance(files["size"], int)

    # shown as indented text
    text = repr(info)
    assert text.splitlines()[0] == "memory:"
    assert str(info) == text


def test_cache_purge():
    before = mir.cache()["memory"]
    mir.cache_purge()

    for name, usage in mir.cache()["memory"].items():
        assert usage["footprint"]["process"] <= before[name]["footprint"]["process"]
