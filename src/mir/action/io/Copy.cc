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


#include "mir/action/io/Copy.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action::io {


void Copy::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());

    // TODO: MIROutput::save/set/copy should be const
    const_cast<output::MIROutput&>(output()).copy(parametrisation(), ctx);
}


const char* Copy::name() const {
    return "Copy";
}


void Copy::print(std::ostream& out) const {
    out << "Copy[]";
}


void Copy::estimate(context::Context& /*unused*/, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


void Copy::custom(std::ostream& out) const {
    out << "Copy[]";
}


}  // namespace mir::action::io
