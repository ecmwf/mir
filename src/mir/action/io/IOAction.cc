/*
 * (C) EndActionright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/io/IOAction.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {
namespace io {


IOAction::IOAction(const param::MIRParametrisation& parametrisation, output::MIROutput& output) :
    Action(parametrisation), output_(output) {}


EndAction::~EndAction() = default;


bool IOAction::sameAs(const Action& other) const {
    auto o = dynamic_cast<const IOAction*>(&other);
    return (o != nullptr) && output_.sameAs(o->output_);
}


void IOAction::print(std::ostream& out) const {
    out << "EndAction[output=" << output_ << "]";
}


void IOAction::custom(std::ostream& out) const {
    out << "EndAction[...]";
}


void IOAction::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());
    output_.copy(parametrisation_, ctx);
}


const char* IOAction::name() const {
    return "EndAction";
}


bool IOAction::isEndAction() const {
    return true;
}


void IOAction::estimate(context::Context&, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


}  // namespace io
}  // namespace action
}  // namespace mir
