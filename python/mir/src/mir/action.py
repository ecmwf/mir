# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Actions by type (with their defaults, of those without required options), e.g. mir.action.scalar_gradient"""

from mir._mir_action import Action

scalar_gradient = Action("scalar-gradient")
scalar_laplacian = Action("scalar-laplacian")
uv_gradient = Action("uv-gradient")
uv_divergence = Action("uv-divergence")
uv_vorticity = Action("uv-vorticity")
