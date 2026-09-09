// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/other/None.h"

#include <ostream>


namespace mir::repres::other {


None::None() = default;


None::None(const param::MIRParametrisation& /*unused*/) {}


None::~None() = default;


void None::print(std::ostream& out) const {
    out << "None[]";
}


static const RepresentationBuilder<None> builder("none");


}  // namespace mir::repres::other
