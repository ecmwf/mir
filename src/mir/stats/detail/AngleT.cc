// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/stats/detail/AngleT.h"


namespace mir::stats::detail {


constexpr double D360 = 360;
constexpr double D180 = 180;
constexpr double TWO  = 2;


/// Angle statistics in degrees [0,360[ (range from std::arg(std::complex))
template <>
AngleT<double, AngleScale::DEGREE, AngleSpace::ASYMMETRIC>::AngleT() :
    rescale_(M_1_PI * D180), descale_(M_PI / D180), globe_(D360), min_(0) {}


/// Angle statistics in degrees [-180,180] (range from std::arg(std::complex))
template <>
AngleT<double, AngleScale::DEGREE, AngleSpace::SYMMETRIC>::AngleT() :
    rescale_(M_1_PI * D180), descale_(M_PI / D180), globe_(D360), min_(-D180) {}


/// Angle statistics in radians [0,2π[ (range from std::arg(std::complex))
template <>
AngleT<double, AngleScale::RADIAN, AngleSpace::ASYMMETRIC>::AngleT() :
    rescale_(1.), descale_(1.), globe_(TWO * M_PI), min_(0) {}


/// Angle statistics in radians [-π,π] (range from std::arg(std::complex))
template <>
AngleT<double, AngleScale::RADIAN, AngleSpace::SYMMETRIC>::AngleT() :
    rescale_(1.), descale_(1.), globe_(TWO * M_PI), min_(-M_PI) {}


}  // namespace mir::stats::detail
