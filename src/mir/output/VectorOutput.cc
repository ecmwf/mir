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


#include "mir/output/VectorOutput.h"

#include <iostream>
#include <typeinfo>  // bad_cast exception

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/VectorInput.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace output {


VectorOutput::VectorOutput(MIROutput& component1, MIROutput& component2) :
    component1_(component1), component2_(component2) {}

VectorOutput::~VectorOutput() = default;


size_t VectorOutput::copy(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& input = ctx.input();

    try {
        auto& v     = dynamic_cast<input::VectorInput&>(input);
        size_t size = 0;

        context::Context ctx1(v.component1_, ctx.statistics());
        size += component1_.copy(param, ctx1);

        context::Context ctx2(v.component2_, ctx.statistics());
        size += component2_.copy(param, ctx2);

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "VectorOutput::copy() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


size_t VectorOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& field = ctx.field();
    auto& input = ctx.input();

    ASSERT(field.dimensions() == 2);

    try {
        auto& vectorInput = dynamic_cast<input::VectorInput&>(input);
        size_t size       = 0;

        context::Context uCtx(vectorInput.component1_, ctx.statistics());
        data::MIRField u(field.representation(), field.hasMissing(), field.missingValue());
        u.update(field.direct(0), 0);
        u.metadata(0, field.metadata(0));
        uCtx.field(u);

        size += component1_.save(param, uCtx);

        context::Context vCtx(vectorInput.component2_, ctx.statistics());
        data::MIRField v(field.representation(), field.hasMissing(), field.missingValue());
        v.update(field.direct(1), 0);
        v.metadata(0, field.metadata(1));
        vCtx.field(v);

        size += component2_.save(param, vCtx);

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "VectorOutput::save() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


size_t VectorOutput::set(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& input = ctx.input();

    try {
        auto& v     = dynamic_cast<input::VectorInput&>(input);
        size_t size = 0;

        context::Context ctx1(v.component1_, ctx.statistics());
        size += component1_.set(param, ctx1);

        context::Context ctx2(v.component2_, ctx.statistics());
        size += component2_.set(param, ctx2);

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "VectorOutput::set() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


bool VectorOutput::sameAs(const MIROutput& other) const {
    auto o = dynamic_cast<const VectorOutput*>(&other);
    return (o != nullptr) && component1_.sameAs(o->component1_) && component2_.sameAs(o->component2_);
}


bool VectorOutput::sameParametrisation(const param::MIRParametrisation& param1,
                                       const param::MIRParametrisation& param2) const {
    return component1_.sameParametrisation(param1, param2) && component2_.sameParametrisation(param1, param2);
}


bool VectorOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    return component1_.printParametrisation(out, param);
}


void VectorOutput::prepare(const param::MIRParametrisation& parametrisation, action::ActionPlan& plan,
                           input::MIRInput& input, output::MIROutput& output) {
    component1_.prepare(parametrisation, plan, input, output);
    component2_.prepare(parametrisation, plan, input, output);
}


void VectorOutput::print(std::ostream& out) const {
    out << "VectorOutput[" << component1_ << "," << component2_ << "]";
}


}  // namespace output
}  // namespace mir
