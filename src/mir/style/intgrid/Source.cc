/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/intgrid/Source.h"

#include <iostream>
#include "eckit/memory/ScopedPtr.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/style/resol/IntermediateGrid.h"


namespace mir {
namespace style {
namespace intgrid {


static IntgridBuilder< Source > __intgrid1("source");
static IntgridBuilder< Source > __intgrid2("SOURCE");


Source::Source(const param::MIRParametrisation& parametrisation, long) :
    style::Intgrid(parametrisation) {

    // both spectral-order and spectral-intermediate-grid are hardcoded because
    // they are different from DefaultParametrisation

    param::RuntimeParametrisation runtime(parametrisation_);
    runtime.set("spectral-order", "cubic");

    eckit::ScopedPtr<param::MIRParametrisation> intermediateGrid(resol::IntermediateGridFactory::build("octahedral-gaussian", runtime));
    ASSERT(intermediateGrid);

    ASSERT(intermediateGrid->get("gridname", gridname_));
    ASSERT(!gridname_.empty());
}


std::string Source::gridname() const {
    return gridname_;
}


void Source::print(std::ostream& out) const {
    out << "Source[gridname=" << gridname_ << "]";
}


}  // namespace intgrid
}  // namespace style
}  // namespace mir

