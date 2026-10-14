# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Interpolations by type (with their defaults), e.g. mir.interpolation.linear, mir.interpolation.k_nn"""

from mir._mir_interpolation import Interpolation

linear = Interpolation("linear")
bilinear = Interpolation("bilinear")
grid_box_average = Interpolation("grid-box-average")
grid_box_statistics = Interpolation("grid-box-statistics")
healpix_resample = Interpolation("healpix-resample")
healpix_statistics = Interpolation("healpix-statistics")
nn = Interpolation("nn")
k_nn = Interpolation("k-nn")
voronoi_statistics = Interpolation("voronoi-statistics")
structured_bilinear = Interpolation("structured-bilinear")
structured_bicubic = Interpolation("structured-bicubic")
structured_biquasicubic = Interpolation("structured-biquasicubic")
