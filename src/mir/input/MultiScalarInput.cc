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


#include "mir/input/MultiScalarInput.h"

#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


MultiScalarInput::MultiScalarInput() = default;


MultiScalarInput::~MultiScalarInput() {
    for (auto c = components_.rbegin(); c != components_.rend(); ++c) {
        delete *c;
    }
}


size_t MultiScalarInput::dimensions() const {
    return components_.size();
}


void MultiScalarInput::appendScalarInput(MIRInput* in) {
    ASSERT(in);
    for (const auto& c : components_) {
        // This is because MIRInput object keeps some state
        ASSERT(c != in);
    }
    components_.push_back(in);
}


const param::MIRParametrisation& MultiScalarInput::parametrisation(size_t which) const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    ASSERT(which < dimensions());
    return components_[which]->parametrisation();
}


grib_handle* MultiScalarInput::gribHandle(size_t which) const {
    // Assumes that both component (e.g. U and V) have the same parametrisation
    ASSERT(which < dimensions());
    return components_[which]->gribHandle();
}


data::MIRField MultiScalarInput::field() const {
    ASSERT(!components_.empty());

    // Assumes that all component share the same parametrisation
    data::MIRField field = components_[0]->field();

    size_t which = 1;
    for (auto c = components_.begin() + 1; c != components_.end(); ++c, ++which) {
        data::MIRField another = (*c)->field();

        ASSERT(another.dimensions() == 1);
        field.update(another.direct(0), which);
    }

    return field;
}


bool MultiScalarInput::next() {
    bool first = true;
    bool next  = false;

    for (auto& c : components_) {
        if (first) {
            next  = c->next();
            first = false;
        }
        else {
            ASSERT(next == c->next());
        }
    }

    return next;
}


bool MultiScalarInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const MultiScalarInput*>(&other);

    if ((o == nullptr) || dimensions() != o->dimensions()) {
        return false;
    }

    for (auto c1 = components_.begin(), c2 = o->components_.begin(); c1 != components_.end(); ++c1, ++c2) {
        if ((*c1)->sameAs(*(*c2))) {
            return false;
        }
    }

    return true;
}


void MultiScalarInput::print(std::ostream& out) const {
    out << "MultiScalarInput[";

    auto sep = "";
    for (auto& c : components_) {
        out << sep << *c;
        sep = ",";
    }

    out << "]";
}


}  // namespace input
}  // namespace mir
