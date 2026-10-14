# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import pytest

import mir

SPECS = [
    dict(type="crop", area=[10, -10, -10, 10]),
    dict(type="mask", area=[60, 0, 40, 30]),
    dict(type="bandpass", bandpass=[0, 10, 0, 20], truncation=63),
    dict(type="limiter", minimum=0, maximum=1),
    dict(type="scalar-gradient"),
    dict(type="scalar-laplacian"),
    dict(type="uv-gradient"),
    dict(type="uv-divergence"),
    dict(type="uv-vorticity"),
]


@pytest.mark.parametrize("spec", SPECS)
def test_action(spec):
    action = mir.Action(spec)
    assert action.type == (action.spec if isinstance(action.spec, str) else action.spec["type"])

    # the same action, from its spec
    assert mir.Action(action.json) == action
    assert mir.Action(**spec) == action


def test_action_spec():
    crop = mir.Action(type="crop", area=[10, -10, -10, 10])
    assert crop.spec == dict(type="crop.area", area=[10, -10, -10, 10])
    assert crop != mir.Action(type="crop", area=[10, -10, -10, 20])
    assert crop != mir.Action(type="mask", area=[10, -10, -10, 10])

    limiter = mir.Action(type="limiter", minimum=0, maximum=1)
    assert limiter.spec == {"type": "filter.limiter", "limiter-minimum": 0, "limiter-maximum": 1}

    # the spec is the type alone, if built from it
    assert mir.Action("scalar-gradient").spec == "filter.scalar-gradient"


def test_action_unsupported():
    with pytest.raises(ValueError):
        mir.Action("unknown")

    with pytest.raises(RuntimeError):
        mir.Action(type="crop")  # missing area


@pytest.mark.parametrize("type", ["scalar-gradient", "scalar-laplacian", "uv-gradient", "uv-divergence", "uv-vorticity"])
def test_action_spec_forms(type):
    from yaml import dump

    # by type alone (its defaults), or with options (as dict, YAML or JSON)
    simple = mir.Action(type)
    assert simple == mir.Action(dict(type=type)) == mir.Action(f"{{type: {type}}}")

    options = dict(nabla_poles_missing_values=True)
    special = mir.Action(type=type, **options)
    assert special != simple
    assert special == mir.Action(dump(dict(type=type, **options), default_flow_style=True, width=1e9))
    assert special == mir.Action(special.json)


@pytest.mark.parametrize("spec", [s for s in SPECS if s["type"] in ("crop", "mask", "bandpass", "limiter")])
def test_action_spec_forms_with_required_options(spec):
    from yaml import dump

    # options are required (as dict, YAML or JSON)
    action = mir.Action(spec)
    assert action == mir.Action(dump(spec, default_flow_style=True, width=1e9))
    assert action == mir.Action(action.json)


@pytest.mark.parametrize("type", ["scalar-gradient", "scalar-laplacian", "uv-gradient", "uv-divergence", "uv-vorticity"])
def test_action_pre_built(type):
    assert getattr(mir.action, type.replace("-", "_")) == mir.Action(type)
