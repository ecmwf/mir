/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/resol/ArchivedValue.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/style/Intgrid.h"


namespace mir {
namespace style {
namespace resol {


static ResolBuilder< ArchivedValue > __resol1("archived-value");
static ResolBuilder< ArchivedValue > __resol2("av");
static ResolBuilder< ArchivedValue > __resol3("AV");


ArchivedValue::ArchivedValue(const param::MIRParametrisation& parametrisation) :
    Resol(parametrisation) {
    const eckit::ScopedPtr<Intgrid> source(IntgridFactory::build("source", parametrisation_, 0));

    gridname_ = source->gridname();
    ASSERT(gridname_.length());
}


void ArchivedValue::prepare(action::ActionPlan& plan) const {

    bool vod2uv = false;
    parametrisation_.userParametrisation().get("vod2uv", vod2uv);

    if (vod2uv) {
        plan.add("transform.sh-vod-to-uv-namedgrid", "gridname", gridname_);
    } else {
        plan.add("transform.sh-scalar-to-namedgrid", "gridname", gridname_);
    }
}


bool ArchivedValue::resultIsSpectral() const {
    return false;
}


void ArchivedValue::print(std::ostream& out) const {
    out << "ArchivedValue[gridname=" << gridname_ << "]";
}


}  // namespace resol
}  // namespace style
}  // namespace mir

