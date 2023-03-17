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
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action::io {


Copy::Copy(const param::MIRParametrisation& parametrisation, output::MIROutput& output) :
    EndAction(parametrisation, output) {}


Copy::~Copy() = default;


void Copy::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());

    // TODO: MIROutput::save/set/copy should be const
    const_cast<output::MIROutput&>(output()).copy(parametrisation_, ctx);
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
