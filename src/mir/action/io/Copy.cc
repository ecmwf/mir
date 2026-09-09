// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/io/Copy.h"

#include <ostream>

#include "mir/action/context/Context.h"
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


void Copy::custom(std::ostream& out) const {
    out << "Copy[]";
}


}  // namespace mir::action::io
