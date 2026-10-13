#!/usr/bin/env python3

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import mir

# by type alone (its defaults): its spec is the type, also pre-built (mir.interpolation.<type>)
linear = mir.Interpolation("linear")
assert linear == mir.Interpolation("{type: linear}") == mir.interpolation.linear
print(linear.spec)  # linear

# with options (as keywords, dict, or YAML/JSON string; keys with underscores or hyphens): its spec lists every
# non-default option, and builds an equal interpolation
k_nn = mir.Interpolation(type="k-nn", k=8, nearest_method="distance-and-nclosest", distance=100000)
assert k_nn != mir.interpolation.k_nn
assert k_nn == mir.Interpolation("{type: k-nn, k: 8, nearest-method: distance-and-nclosest, distance: 100000}")
assert k_nn == mir.Interpolation(k_nn.spec) == mir.Interpolation(k_nn.json)
print(k_nn.spec)  # {'type': 'k-nearest', ..., 'nclosest': 8, 'nearest-method': 'distance-and-nclosest', ...}

# on a job: as object, spec, or type and options (interpolation options are set one way, by spec or by option)
job = mir.Job(grid=[1, 1], interpolation=k_nn)
job = mir.Job(grid=[1, 1], interpolation="{type: k-nn, k: 8}")
job = mir.Job(grid=[1, 1], interpolation="k-nn", nclosest=8)
print(job.json)

# all types, by type alone or with options
for interpolation in [
    mir.interpolation.linear,
    mir.interpolation.bilinear,
    mir.interpolation.grid_box_average,
    mir.Interpolation(type="grid-box-statistics", statistics="mode-integral"),
    mir.interpolation.healpix_resample,
    mir.Interpolation(type="healpix-statistics", statistics="mean"),
    mir.interpolation.nn,
    mir.Interpolation(type="k-nn", k=8),
    mir.Interpolation(type="voronoi-statistics", statistics="minimum"),
    mir.interpolation.structured_bilinear,
    mir.interpolation.structured_bicubic,
    mir.interpolation.structured_biquasicubic,
]:
    assert mir.Interpolation(interpolation.json) == interpolation
    print(interpolation.type, interpolation.spec)
