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


#include "mir/style/resol/IntermediateGaussianGrid.h"


namespace mir {
namespace style {
namespace resol {


namespace {
struct f_grid_t {};
struct n_grid_t {};
struct o_grid_t {};
static IntermediateGridBuilder< IntermediateGaussianGrid<f_grid_t> > __intermediate_grid_f("regular-gaussian");
static IntermediateGridBuilder< IntermediateGaussianGrid<n_grid_t> > __intermediate_grid_n("classic-gaussian");
static IntermediateGridBuilder< IntermediateGaussianGrid<o_grid_t> > __intermediate_grid_o("octahedral-gaussian");
}


template<> std::string IntermediateGaussianGrid<f_grid_t>::gaussianGridTypeLetter() const { return "F"; }
template<> std::string IntermediateGaussianGrid<n_grid_t>::gaussianGridTypeLetter() const { return "N"; }
template<> std::string IntermediateGaussianGrid<o_grid_t>::gaussianGridTypeLetter() const { return "O"; }


}  // namespace resol
}  // namespace style
}  // namespace mir

