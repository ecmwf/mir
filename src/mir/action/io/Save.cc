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


Save::Save(const param::MIRParametrisation& parametrisation, input::MIRInput& input, output::MIROutput& output) :
    Action(parametrisation), input_(input), output_(output) {}


Save::~Save() = default;


bool Save::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Save*>(&other);
    return (o != nullptr) && input_.sameAs(o->input_) && output_.sameAs(o->output_) &&
           o->output_.sameParametrisation(parametrisation_, o->parametrisation_);
}


void Save::print(std::ostream& out) const {
    out << "Save[";
    if (output_.printParametrisation(out, parametrisation_)) {
        out << ",";
    }
    out << "output=" << output_ << "]";
}


void Save::custom(std::ostream& out) const {
    out << "Save[";
    if (output_.printParametrisation(out, parametrisation_)) {
        out << ",";
    }
    out << "output=...]";
}


void Save::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());
    output_.save(parametrisation_, ctx);
}


const char* Save::name() const {
    return "Save";
}


bool Save::isEndAction() const {
    return true;
}

void Save::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    output_.estimate(parametrisation_, estimation, ctx);
}


}  // namespace io
}  // namespace action
}  // namespace mir
