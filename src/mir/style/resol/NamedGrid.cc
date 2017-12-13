/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/resol/NamedGrid.h"

#include "eckit/exception/Exceptions.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {
namespace resol {


NamedGrid::NamedGrid(const std::string& gridname, const param::MIRParametrisation& parametrisation) :
    Resol(parametrisation),
    gridname_(gridname) {
    ASSERT(!gridname_.empty());
}


void NamedGrid::prepare(action::ActionPlan& plan) const {

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-uv-namedgrid", "gridname", gridname_);
    } else {
        plan.add("transform.sh-scalar-to-namedgrid", "gridname", gridname_);
    }
}


bool NamedGrid::resultIsSpectral() const {
    return false;
}


void NamedGrid::print(std::ostream& out) const {
    out << "NamedGrid[gridname=" << gridname_ << "]";
}




}  // namespace resol
}  // namespace style
}  // namespace mir

