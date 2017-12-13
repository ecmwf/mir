/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/truncation/Truncation.h"

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {
namespace truncation {


Truncation::Truncation(long truncation, const param::MIRParametrisation& parametrisation) :
    style::Truncation(parametrisation),
    truncation_(truncation) {
    ASSERT(truncation_ > 0);
}


long Truncation::truncation() const {
    return truncation_;
}


void Truncation::print(std::ostream& out) const {
    out << "Truncation[truncation=" << truncation_ << "]";
}


}  // namespace truncation
}  // namespace style
}  // namespace mir

