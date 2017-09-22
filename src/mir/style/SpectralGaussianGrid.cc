/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/style/SpectralGaussianGrid.h"


namespace mir {
namespace style {


namespace {
struct f_grid_t {};
struct n_grid_t {};
struct o_grid_t {};
static SpectralGridBuilder< SpectralGaussianGrid<f_grid_t> > __spectral_f_grid("regular-gaussian");
static SpectralGridBuilder< SpectralGaussianGrid<n_grid_t> > __spectral_n_grid("classic-gaussian");
static SpectralGridBuilder< SpectralGaussianGrid<o_grid_t> > __spectral_o_grid("octahedral-gaussian");
}


template<> std::string SpectralGaussianGrid<f_grid_t>::gaussianGridTypeLetter() const { return "F"; }
template<> std::string SpectralGaussianGrid<n_grid_t>::gaussianGridTypeLetter() const { return "N"; }
template<> std::string SpectralGaussianGrid<o_grid_t>::gaussianGridTypeLetter() const { return "O"; }


}  // namespace style
}  // namespace mir

