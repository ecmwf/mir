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


#include "mir/action/io/Set.h"

#include "mir/action/context/Context.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action::io {


Set::Set(const param::MIRParametrisation& parametrisation, output::MIROutput& output) :
    EndAction(parametrisation, output) {}


Set::~Set() = default;


void Set::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());

    // TODO: MIROutput::save/set/copy should be const
    const_cast<output::MIROutput&>(output()).set(parametrisation_, ctx);
}


const char* Set::name() const {
    return "Set";
}


}  // namespace mir::action::io
