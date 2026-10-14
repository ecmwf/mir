# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import pytest

import mir


@pytest.mark.parametrize(
    "spec, type",
    [
        (dict(type="linear"), "linear"),
        (dict(type="bilinear"), "bilinear"),
        (dict(type="grid-box-average"), "grid-box-average"),
        (dict(type="grid-box-statistics", statistics="mode-integral"), "grid-box-statistics"),
        (dict(type="healpix-resample"), "healpix-resample"),
        (dict(type="healpix-statistics", statistics="mean"), "healpix-statistics"),
        (dict(type="nn"), "nearest-neighbour"),
        (dict(type="k-nn", k=8), "k-nearest"),
        (dict(type="voronoi-statistics", statistics="minimum"), "voronoi-statistics"),
        (dict(type="structured-bilinear"), "structured-bilinear"),
        (dict(type="structured-bicubic"), "structured-bicubic"),
        (dict(type="structured-biquasicubic"), "structured-biquasicubic"),
    ],
)
def test_interpolation(spec, type):
    interpolation = mir.Interpolation(spec)
    assert interpolation.type == type

    # the same interpolation, from its spec
    assert mir.Interpolation(interpolation.json) == interpolation
    assert mir.Interpolation(**spec) == interpolation


def test_interpolation_options():
    # statistics (*-statistics) and k (k-nn), as mir keys
    stats = mir.Interpolation(type="grid-box-statistics", statistics="mode-integral")
    assert stats.spec["interpolation-statistics"] == "mode-integral"
    assert stats != mir.Interpolation(type="grid-box-statistics", statistics="maximum")

    knn = mir.Interpolation(type="k-nn", k=8)
    assert knn.spec["nclosest"] == 8


def test_interpolation_type_alone():
    # the spec is the type alone, if built from it
    for type in ("linear", "bilinear", "grid-box-average", "structured-bilinear", "structured-bicubic"):
        assert mir.Interpolation(type).spec == type

    assert mir.Interpolation("nn").spec == "nearest-neighbour"


def test_interpolation_unsupported():
    with pytest.raises(ValueError):
        mir.Interpolation("matrix")

    with pytest.raises(ValueError):
        mir.Interpolation(type="grid-box-statistics", statistics="unknown")


# each type with a (non-default) option
OPTIONS = {
    "linear": dict(finite_element_validate_mesh=True),
    "bilinear": dict(finite_element_validate_mesh=True),
    "grid-box-average": dict(non_linear=["missing-if-all-missing"]),
    "grid-box-statistics": dict(statistics="minimum"),
    "healpix-resample": dict(non_linear=["missing-if-all-missing"]),
    "healpix-statistics": dict(statistics="minimum"),
    "nn": dict(k=5),
    "k-nn": dict(k=8),
    "voronoi-statistics": dict(statistics="mean"),
    "structured-bilinear": dict(interpolation_matrix_free=False),
    "structured-bicubic": dict(interpolation_matrix_free=False),
    "structured-biquasicubic": dict(interpolation_matrix_free=False),
}


@pytest.mark.parametrize("type, options", OPTIONS.items())
def test_interpolation_spec_forms(type, options):
    from yaml import dump

    # by type alone (its defaults), or with options (as dict, YAML or JSON)
    simple = mir.Interpolation(type)
    assert simple == mir.Interpolation(dict(type=type)) == mir.Interpolation(f"{{type: {type}}}")

    special = mir.Interpolation(type=type, **options)
    assert special != simple
    assert special == mir.Interpolation(dict(type=type, **options))
    assert special == mir.Interpolation(dump(dict(type=type, **options), default_flow_style=True, width=1e9))
    assert special == mir.Interpolation(special.json)


EXAMPLES = [
    ("linear", dict(finite_element_validate_mesh=True)),
    ("k-nn", dict(k=8, nearest_method="distance-and-nclosest", distance=100000)),
]


@pytest.mark.parametrize("type, options", EXAMPLES)
def test_interpolation_by_example(type, options):
    import json

    from yaml import dump

    # bare name: the interpolation with its defaults, its spec is the name (as mir)
    simple = mir.Interpolation(type)
    assert simple.spec == simple.type

    # {type: name}: exactly the same interpolation (also pre-built)
    assert mir.Interpolation(f"{{type: {type}}}") == simple
    assert getattr(mir.interpolation, type.replace("-", "_")) == simple

    # with options: a different interpolation, its spec lists every non-default option, and rebuilds an equal one
    special = mir.Interpolation(type=type, **options)
    assert special != simple
    assert isinstance(special.spec, dict)
    assert mir.Interpolation(special.spec) == special

    # as keywords, a dict, YAML or JSON; option keys with underscores or hyphens
    hyphens = {k.replace("_", "-"): v for k, v in options.items()}
    for spec in (
        dict(type=type, **options),
        dict(type=type, **hyphens),
        dump(dict(type=type, **hyphens), default_flow_style=True, width=1e9),
        json.dumps(dict(type=type, **options)),
        special.json,
    ):
        assert mir.Interpolation(spec) == special


def test_job_interpolation_by_example():
    import json

    def job_interpolation(job):
        return mir.Interpolation(json.loads(job.json))

    # the same interpolation, set on a job as object, spec (dict, YAML or JSON), or name and options
    k_nn = mir.Interpolation(type="k-nn", k=8)
    for value in (k_nn, k_nn.spec, k_nn.json, dict(type="k-nn", k=8), "{type: k-nn, k: 8}"):
        assert job_interpolation(mir.Job(interpolation=value)) == k_nn

    assert job_interpolation(mir.Job(interpolation="k-nn", nclosest=8)) == k_nn
    assert job_interpolation(mir.Job(interpolation=mir.interpolation.k_nn)) == mir.interpolation.k_nn

    # interpolation options are set one way, by spec or by option
    with pytest.raises(RuntimeError):
        mir.Job(interpolation=k_nn, nclosest=4)
    with pytest.raises(RuntimeError):
        mir.Job(nclosest=4, interpolation=k_nn)


@pytest.mark.parametrize("type", list(OPTIONS))
def test_interpolation_pre_built(type):
    assert getattr(mir.interpolation, type.replace("-", "_")) == mir.Interpolation(type)
