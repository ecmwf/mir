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


#include "mir/style/IntermediateGaussianGrid.h"


namespace mir {
namespace style {


namespace {


struct regular_gg_t {};
struct reduced_gg_t {};
struct octahedral_gg_t {};


static IntermediateGridBuilder< IntermediateGaussianGrid<regular_gg_t> >    __intermediate_regular_gg("regular");
static IntermediateGridBuilder< IntermediateGaussianGrid<reduced_gg_t> >    __intermediate_reduced_gg("reduced");
static IntermediateGridBuilder< IntermediateGaussianGrid<octahedral_gg_t> > __intermediate_octahedral_gg("octahedral");


}


template<> std::string IntermediateGaussianGrid<regular_gg_t>::gaussianGridType()    const { return "regular-gg"; }
template<> std::string IntermediateGaussianGrid<reduced_gg_t>::gaussianGridType()    const { return "reduced-gg"; }
template<> std::string IntermediateGaussianGrid<octahedral_gg_t>::gaussianGridType() const { return "octahedral-gg"; }


}  // namespace style
}  // namespace mir

