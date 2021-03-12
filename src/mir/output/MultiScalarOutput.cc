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


#include "mir/output/MultiScalarOutput.h"

#include <iostream>
#include <typeinfo>  // bad_cast exception

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/MultiScalarInput.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace output {


MultiScalarOutput::MultiScalarOutput() = default;


MultiScalarOutput::~MultiScalarOutput() {
    for (auto c = components_.rbegin(); c != components_.rend(); ++c) {
        delete *c;
    }
}


void MultiScalarOutput::appendScalarOutput(MIROutput* out) {
    components_.push_back(out);
}


size_t MultiScalarOutput::copy(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& input = ctx.input();

    try {
        auto& multi  = dynamic_cast<input::MultiScalarInput&>(input);
        size_t size  = 0;
        size_t count = 0;

        for (auto& c : components_) {
            context::Context componentCtx(*(multi.components_[count++]), ctx.statistics());
            size += c->copy(param, componentCtx);
        }

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "MultiScalarOutput::copy() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


size_t MultiScalarOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& field = ctx.field();
    auto& input = ctx.input();

    ASSERT(field.dimensions() > 0);

    try {
        auto& multi  = dynamic_cast<input::MultiScalarInput&>(input);
        size_t size  = 0;
        size_t count = 0;

        for (auto& c : components_) {
            context::Context componentCtx(*(multi.components_[count]), ctx.statistics());

            data::MIRField u(field.representation(), field.hasMissing(), field.missingValue());
            u.update(field.direct(count), 0);
            u.metadata(0, field.metadata(0));
            componentCtx.field(u);

            size += c->save(param, componentCtx);
            count++;
        }

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "MultiScalarOutput::save() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


size_t MultiScalarOutput::set(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& field = ctx.field();
    auto& input = ctx.input();

    ASSERT(field.dimensions() > 0);

    try {
        auto& multi  = dynamic_cast<input::MultiScalarInput&>(input);
        size_t size  = 0;
        size_t count = 0;

        for (auto& c : components_) {
            context::Context componentCtx(*(multi.components_[count]), ctx.statistics());

            data::MIRField u(field.representation(), field.hasMissing(), field.missingValue());
            u.update(field.direct(count), 0);
            u.metadata(0, field.metadata(0));
            componentCtx.field(u);

            size += c->set(param, componentCtx);
            count++;
        }

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "MultiScalarOutput::set() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


bool MultiScalarOutput::sameAs(const MIROutput& other) const {
    auto o = dynamic_cast<const MultiScalarOutput*>(&other);

    if ((o == nullptr) || (components_.size() != o->components_.size())) {
        return false;
    }

    for (auto c1 = components_.begin(), c2 = o->components_.begin(); c1 != components_.end(); ++c1, ++c2) {
        if ((*c1)->sameAs(*(*c2))) {
            return false;
        }
    }

    return true;
}


bool MultiScalarOutput::sameParametrisation(const param::MIRParametrisation& param1,
                                            const param::MIRParametrisation& param2) const {

    for (auto& c : components_) {
        if (!(c->sameParametrisation(param1, param2))) {
            return false;
        }
    }

    return true;
}


bool MultiScalarOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    ASSERT(!components_.empty());
    return components_[0]->printParametrisation(out, param);
}


void MultiScalarOutput::prepare(const param::MIRParametrisation& parametrisation, action::ActionPlan& plan,
                                input::MIRInput& input, MIROutput& output) {
    ASSERT(!components_.empty());
    for (auto& c : components_) {
        c->prepare(parametrisation, plan, input, output);
    }
}


void MultiScalarOutput::print(std::ostream& out) const {
    out << "MultiScalarOutput[";

    const char* sep = "";
    for (auto& c : components_) {
        out << sep << c;
        sep = ",";
    }

    out << "]";
}


}  // namespace output
}  // namespace mir
