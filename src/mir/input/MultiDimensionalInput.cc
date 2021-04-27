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


#include "mir/input/MultiDimensionalInput.h"

#include <algorithm>
#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


MultiDimensionalInput::MultiDimensionalInput() = default;


MultiDimensionalInput::~MultiDimensionalInput() {
    for (auto c = components_.rbegin(); c != components_.rend(); ++c) {
        delete *c;
    }
}


size_t MultiDimensionalInput::dimensions() const {
    return components_.size();
}


void MultiDimensionalInput::append(MIRInput* in) {
    ASSERT(in);
    for (const auto& c : components_) {
        // Because MIRInput keeps some state
        ASSERT(c != in);
    }
    components_.push_back(in);
}


const param::MIRParametrisation& MultiDimensionalInput::parametrisation(size_t which) const {
    // Assumes all components have the same parametrisation
    return components_.at(which)->parametrisation();
}


grib_handle* MultiDimensionalInput::gribHandle(size_t which) const {
    // Assumes all components have the same parametrisation
    return components_.at(which)->gribHandle();
}


data::MIRField MultiDimensionalInput::field() const {
    ASSERT(!components_.empty());

    // Assumes all components have the same parametrisation
    auto field = components_[0]->field();

    size_t which = 1;
    for (auto c = components_.begin() + 1; c != components_.end(); ++c, ++which) {
        ASSERT(*c != nullptr);
        auto another = (*c)->field();

        ASSERT(another.dimensions() == 1);
        field.update(another.direct(0), which);
    }

    return field;
}


bool MultiDimensionalInput::next() {
    for (auto& c : components_) {
        ASSERT(c != nullptr);
        if (!c->next()) {
            delete c;
            c = nullptr;
        }
    }

    components_.erase(std::remove_if(components_.begin(), components_.end(), [](MIRInput* c) { return c == nullptr; }),
                      components_.end());

    return !components_.empty();
}


bool MultiDimensionalInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const MultiDimensionalInput*>(&other);

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


void MultiDimensionalInput::print(std::ostream& out) const {
    out << "MultiDimensionalInput[";

    auto sep = "";
    for (auto& c : components_) {
        out << sep << *c;
        sep = ",";
    }

    out << "]";
}


}  // namespace input
}  // namespace mir
