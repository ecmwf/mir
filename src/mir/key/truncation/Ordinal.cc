// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
