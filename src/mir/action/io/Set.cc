// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/io/Set.h"

#include "mir/action/context/Context.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action::io {


void Set::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());

    // TODO: MIROutput::save/set/copy should be const
    const_cast<output::MIROutput&>(output()).set(parametrisation(), ctx);
}


const char* Set::name() const {
    return "Set";
}


}  // namespace mir::action::io
