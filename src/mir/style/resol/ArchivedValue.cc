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
#include "mir/action/plan/ActionPlan.h"


namespace mir {
namespace style {
namespace resol {


static ResolBuilder< ArchivedValue > __resol1("archived-value");
static ResolBuilder< ArchivedValue > __resol2("av");
static ResolBuilder< ArchivedValue > __resol3("AV");


ArchivedValue::ArchivedValue(const param::MIRParametrisation& parametrisation) :
    Resol(parametrisation) {

    // Setup intermediate grid before truncation
    // NOTE: truncation can depend on the intermediate grid Gaussian number
    std::string value = "source";
    parametrisation_.userParametrisation().get("intgrid", value);
    intgrid_.reset(IntgridFactory::build(value, parametrisation_, 0));
    ASSERT(intgrid_);
}


void ArchivedValue::prepare(action::ActionPlan& plan) const {

    // transform, if specified
    const std::string gridname = intgrid_->gridname();
    if (!gridname.empty()) {

        bool vod2uv = false;
        parametrisation_.userParametrisation().get("vod2uv", vod2uv);

        if (vod2uv) {
            plan.add("transform.sh-vod-to-uv-namedgrid", "gridname", gridname);
        } else {
            plan.add("transform.sh-scalar-to-namedgrid", "gridname", gridname);
        }
    }
}


bool ArchivedValue::resultIsSpectral() const {
    return intgrid_->gridname().empty();
}


void ArchivedValue::print(std::ostream& out) const {
    out << "ArchivedValue["
            "intgrid=" << *intgrid_
        << "]";
}


}  // namespace resol
}  // namespace style
}  // namespace mir

