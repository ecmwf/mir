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

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {


namespace {
static IntermediateGridBuilder< IntermediateGaussianGrid > __intermediate_gaussian_grid1("octahedral");
}


IntermediateGaussianGrid::IntermediateGaussianGrid(const param::MIRParametrisation& parametrisation) :
    IntermediateGrid(parametrisation) {
}


}  // namespace style
}  // namespace mir

