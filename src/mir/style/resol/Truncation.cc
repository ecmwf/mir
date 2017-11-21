/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/resol/Truncation.h"

#include "eckit/exception/Exceptions.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {
namespace resol {


Truncation::Truncation(long truncation, const param::MIRParametrisation& parametrisation) :
    Resol(parametrisation),
    truncation_(truncation) {
    ASSERT(truncation_ > 0);
}


void Truncation::prepare(action::ActionPlan& plan) const {

    long T = 0;
    ASSERT(parametrisation_.fieldParametrisation().get("truncation", T));
    ASSERT(T > 0);

    if (T > truncation_ ) {
        plan.add("transform.sh-truncate", "truncation", truncation_);
    }
}


bool Truncation::resultIsSpectral() const {
    return true;
}


void Truncation::print(std::ostream& out) const {
    out << "Truncation[truncation=" << truncation_ << "]";
}




}  // namespace resol
}  // namespace style
}  // namespace mir

