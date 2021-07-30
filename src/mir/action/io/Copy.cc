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


namespace mir {
namespace action {
namespace io {


Copy::Copy(const param::MIRParametrisation& parametrisation, output::MIROutput& output) :
    IOAction(parametrisation, output) {}


Copy::~Copy() = default;


bool Copy::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Copy*>(&other);
    return (o != nullptr) && output().sameAs(o->output());
}


void Copy::print(std::ostream& out) const {
    out << "Copy[output=" << output() << "]";
}


void Copy::custom(std::ostream& out) const {
    out << "Copy[...]";
}


void Copy::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());
    const_cast<output::MIROutput&>(output()).copy(parametrisation_, ctx);
}


const char* Copy::name() const {
    return "Copy";
}


void Copy::estimate(context::Context&, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


}  // namespace io
}  // namespace action
}  // namespace mir
