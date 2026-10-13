#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import mir

# by type alone (its defaults): its spec is the (mir) type, also pre-built (mir.action.<type>)
gradient = mir.Action("scalar-gradient")
assert gradient == mir.Action("{type: scalar-gradient}") == mir.action.scalar_gradient
print(gradient.spec)  # filter.scalar-gradient

# with options (as keywords, dict, or YAML/JSON string): its spec lists every non-default option
special = mir.Action(type="scalar-gradient", nabla_poles_missing_values=True)
assert special != gradient
assert special == mir.Action("{type: scalar-gradient, nabla-poles-missing-values: true}") == mir.Action(special.json)
print(special.spec)

# all types (crop, mask, bandpass and limiter require options)
for action in [
    mir.Action(type="crop", area=[10, -10, -10, 10]),  # north, west, south, east
    mir.Action(type="mask", area=[60, 0, 40, 30]),
    mir.Action(type="bandpass", bandpass=[0, 10, 0, 20], truncation=63),  # M and N ranges, of spectral fields
    mir.Action(type="limiter", minimum=0, maximum=1),
    mir.action.scalar_gradient,
    mir.action.scalar_laplacian,
    mir.action.uv_gradient,
    mir.action.uv_divergence,
    mir.action.uv_vorticity,
]:
    assert mir.Action(action.json) == action
    print(action.type, action.spec)
