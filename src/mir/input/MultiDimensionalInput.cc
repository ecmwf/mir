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


namespace mir::input {


MultiDimensionalInput::MultiDimensionalInput() = default;


MultiDimensionalInput::~MultiDimensionalInput() {
    for (auto c = dimensions_.rbegin(); c != dimensions_.rend(); ++c) {
        delete *c;
    }
}


size_t MultiDimensionalInput::dimensions() const {
    return dimensions_.size();
}


void MultiDimensionalInput::append(MIRInput* in) {
    ASSERT(in);
    for (const auto& d : dimensions_) {
        // Because MIRInput keeps some state
        ASSERT(d != in);
    }
    dimensions_.push_back(in);
}


const param::MIRParametrisation& MultiDimensionalInput::parametrisation(size_t which) const {
    // Assumes all components have the same parametrisation
    return dimensions_.at(which)->parametrisation();
}


grib_handle* MultiDimensionalInput::gribHandle(size_t which) const {
    // Assumes all components have the same parametrisation
    return dimensions_.at(which)->gribHandle();
}


void MultiDimensionalInput::setAuxiliaryInformation(const util::ValueMap& map) {
    for (const auto& d : dimensions_) {
        ASSERT(d != nullptr);
        d->setAuxiliaryInformation(map);
    }
}


data::MIRField MultiDimensionalInput::field() const {
    ASSERT(!dimensions_.empty());

    // Assumes all components have the same parametrisation
    auto field = dimensions_[0]->field();

    size_t which = 1;
    for (auto d = dimensions_.begin() + 1; d != dimensions_.end(); ++d, ++which) {
        ASSERT(*d != nullptr);
        auto another = (*d)->field();

        ASSERT(another.dimensions() == 1);
        field.update(another.direct(0), which);
    }

    return field;
}


bool MultiDimensionalInput::next() {
    for (auto& d : dimensions_) {
        ASSERT(d != nullptr);
        if (d->next()) {
            ASSERT(d->dimensions() == 1);
        }
        else {
            delete d;
            d = nullptr;
        }
    }

    dimensions_.erase(std::remove_if(dimensions_.begin(), dimensions_.end(), [](MIRInput* d) { return d == nullptr; }),
                      dimensions_.end());

    return !dimensions_.empty();
}


bool MultiDimensionalInput::sameAs(const MIRInput& other) const {
    const auto* o = dynamic_cast<const MultiDimensionalInput*>(&other);

    if ((o == nullptr) || dimensions() != o->dimensions()) {
        return false;
    }

    for (auto d1 = dimensions_.cbegin(), d2 = o->dimensions_.cbegin(); d1 != dimensions_.cend(); ++d1, ++d2) {
        if (!(*d1)->sameAs(*(*d2))) {
            return false;
        }
    }

    return true;
}


void MultiDimensionalInput::print(std::ostream& out) const {
    out << "MultiDimensionalInput[";

    const auto* sep = "";
    for (const auto& d : dimensions_) {
        out << sep << *d;
        sep = ",";
    }

    out << "]";
}


}  // namespace mir::input
