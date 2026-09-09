// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/truncation/None.h"


namespace mir::key::truncation {


static const TruncationBuilder<None> __truncation1("none");
static const TruncationBuilder<None> __truncation2("NONE");


None::None(const param::MIRParametrisation& parametrisation, long /*unused*/) : Truncation(parametrisation) {}


bool None::truncation(long& /*unused*/, long /*inputTrucation*/) const {
    return false;
}


}  // namespace mir::key::truncation
