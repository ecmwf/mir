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


#include "mir/action/io/Save.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/input/MIRInput.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {
namespace io {


Save::Save(const param::MIRParametrisation& parametrisation, output::MIROutput& output) :
    IOAction(parametrisation, output) {}


Save::~Save() = default;


void Save::print(std::ostream& out) const {
    out << "Save[";
    if (output().printParametrisation(out, parametrisation_)) {
        out << ",";
    }
    out << "output=" << output() << "]";
}


void Save::custom(std::ostream& out) const {
    out << "Save[";
    if (output().printParametrisation(out, parametrisation_)) {
        out << ",";
    }
    out << "output=...]";
}


void Save::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());
    const_cast<output::MIROutput&>(output()).save(parametrisation_, ctx);
}


const char* Save::name() const {
    return "Save";
}


void Save::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    output().estimate(parametrisation_, estimation, ctx);
}


}  // namespace io
}  // namespace action
}  // namespace mir
