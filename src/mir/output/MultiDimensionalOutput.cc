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


#include "mir/output/MultiDimensionalOutput.h"

#include <ostream>
#include <sstream>
#include <typeinfo>  // bad_cast exception

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/MultiDimensionalInput.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace output {


MultiDimensionalOutput::MultiDimensionalOutput() = default;


MultiDimensionalOutput::~MultiDimensionalOutput() {
    for (auto& d : dimensions_) {
        delete d;
    }
}


void MultiDimensionalOutput::appendDimensionalOutput(MIROutput* out) {
    ASSERT(out != nullptr);
    dimensions_.push_back(out);
}


size_t MultiDimensionalOutput::copy(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& input = ctx.input();

    try {
        auto& multi  = dynamic_cast<input::MultiDimensionalInput&>(input);
        size_t size  = 0;
        size_t count = 0;

        for (auto& d : dimensions_) {
            context::Context componentCtx(*(multi.dimensions_[count++]), ctx.statistics());
            size += d->copy(param, componentCtx);
        }

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "MultiDimensionalOutput::copy() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


size_t MultiDimensionalOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& field = ctx.field();
    auto& input = ctx.input();

    ASSERT(field.dimensions() > 0);

    try {
        auto& multi  = dynamic_cast<input::MultiDimensionalInput&>(input);
        size_t size  = 0;
        size_t count = 0;

        for (auto& d : dimensions_) {
            context::Context componentCtx(*(multi.dimensions_[count]), ctx.statistics());

            data::MIRField u(field.representation(), field.hasMissing(), field.missingValue());
            u.update(field.direct(count), 0);
            u.metadata(0, field.metadata(0));
            componentCtx.field(u);

            size += d->save(param, componentCtx);
            count++;
        }

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "MultiDimensionalOutput::save() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


size_t MultiDimensionalOutput::set(const param::MIRParametrisation& param, context::Context& ctx) {
    auto& field = ctx.field();
    auto& input = ctx.input();

    ASSERT(field.dimensions() > 0);

    try {
        auto& multi  = dynamic_cast<input::MultiDimensionalInput&>(input);
        size_t size  = 0;
        size_t count = 0;

        for (auto& d : dimensions_) {
            context::Context componentCtx(*(multi.dimensions_[count]), ctx.statistics());

            data::MIRField u(field.representation(), field.hasMissing(), field.missingValue());
            u.update(field.direct(count), 0);
            u.metadata(0, field.metadata(0));
            componentCtx.field(u);

            size += d->set(param, componentCtx);
            count++;
        }

        return size;
    }
    catch (std::bad_cast&) {
        std::ostringstream os;
        os << "MultiDimensionalOutput::set() not implemented for input of type: " << input;
        throw exception::SeriousBug(os.str());
    }
}


bool MultiDimensionalOutput::sameAs(const MIROutput& other) const {
    const auto* o = dynamic_cast<const MultiDimensionalOutput*>(&other);

    if ((o == nullptr) || (dimensions_.size() != o->dimensions_.size())) {
        return false;
    }

    for (auto d1 = dimensions_.begin(), d2 = o->dimensions_.begin(); d1 != dimensions_.end(); ++d1, ++d2) {
        if ((*d1)->sameAs(*(*d2))) {
            return false;
        }
    }

    return true;
}


bool MultiDimensionalOutput::sameParametrisation(const param::MIRParametrisation& param1,
                                                 const param::MIRParametrisation& param2) const {
    for (const auto& d : dimensions_) {
        if (!(d->sameParametrisation(param1, param2))) {
            return false;
        }
    }

    return true;
}


bool MultiDimensionalOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation& param) const {
    ASSERT(!dimensions_.empty());
    return dimensions_[0]->printParametrisation(out, param);
}


void MultiDimensionalOutput::prepare(const param::MIRParametrisation& parametrisation, action::ActionPlan& plan,
                                     MIROutput& output) {
    ASSERT(!dimensions_.empty());
    for (auto& d : dimensions_) {
        d->prepare(parametrisation, plan, output);
    }
}


void MultiDimensionalOutput::print(std::ostream& out) const {
    out << "MultiDimensionalOutput[";

    const auto* sep = "";
    for (const auto& d : dimensions_) {
        out << sep << d;
        sep = ",";
    }

    out << "]";
}


}  // namespace output
}  // namespace mir
