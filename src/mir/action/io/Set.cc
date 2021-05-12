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

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/input/MIRInput.h"
#include "mir/output/MIROutput.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {
namespace io {


Set::Set(const param::MIRParametrisation& parametrisation, input::MIRInput& input, output::MIROutput& output) :
    Action(parametrisation), input_(input), output_(output) {}


Set::~Set() = default;


bool Set::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Set*>(&other);
    return (o != nullptr) && input_.sameAs(o->input_) && output_.sameAs(o->output_) &&
           o->output_.sameParametrisation(parametrisation_, o->parametrisation_);
}


void Set::print(std::ostream& out) const {
    out << "Set[";
    if (output_.printParametrisation(out, parametrisation_)) {
        out << ",";
    }
    out << "output=" << output_ << "]";
}


void Set::custom(std::ostream& out) const {
    out << "Set[";
    if (output_.printParametrisation(out, parametrisation_)) {
        out << ",";
    }
    out << "output=...]";
}


void Set::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().saveTimer());
    output_.set(parametrisation_, ctx);
}


const char* Set::name() const {
    return "Set";
}


bool Set::isEndAction() const {
    return true;
}

void Set::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    output_.estimate(parametrisation_, estimation, ctx);
}


}  // namespace io
}  // namespace action
}  // namespace mir
