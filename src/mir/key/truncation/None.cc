/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/key/truncation/None.h"


namespace mir {
namespace key {
namespace truncation {


static const TruncationBuilder<None> __truncation1("none");
static const TruncationBuilder<None> __truncation2("NONE");


None::None(const param::MIRParametrisation& parametrisation, long) : Truncation(parametrisation) {}


bool None::truncation(long&, long) const {
    return false;
}


}  // namespace truncation
}  // namespace key
}  // namespace mir
