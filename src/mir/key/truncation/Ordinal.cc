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


#include "mir/key/truncation/Ordinal.h"

#include "mir/util/Exceptions.h"


namespace mir::key::truncation {


Ordinal::Ordinal(long truncation, const param::MIRParametrisation& parametrisation) :
    Truncation(parametrisation), truncation_(truncation) {
    ASSERT(truncation_ > 0);
}


bool Ordinal::truncation(long& T, long /*inputTrucation*/) const {
    T = truncation_;
    return true;
}


}  // namespace mir::key::truncation
