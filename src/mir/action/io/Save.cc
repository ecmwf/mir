// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/io/Save.h"

#include "mir/action/context/Context.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action::io {


void Save::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());

    // TODO: MIROutput::save/set/copy should be const
    const_cast<output::MIROutput&>(output()).save(parametrisation(), ctx);
}


const char* Save::name() const {
    return "Save";
}


}  // namespace mir::action::io
