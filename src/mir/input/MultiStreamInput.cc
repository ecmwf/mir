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


#include "mir/input/MultiStreamInput.h"

#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


MultiStreamInput::MultiStreamInput() = default;


MultiStreamInput::~MultiStreamInput() = default;


void MultiStreamInput::append(MIRInput* in) {
    ASSERT(in);
    streams_.push_back(in);
}


size_t MultiStreamInput::dimensions() const {
    ASSERT(!streams_.empty());
    return streams_.front()->dimensions();
}


const param::MIRParametrisation& MultiStreamInput::parametrisation(size_t which) const {
    ASSERT(!streams_.empty());
    return streams_.front()->parametrisation(which);
}


data::MIRField MultiStreamInput::field() const {
    ASSERT(!streams_.empty());
    return streams_.front()->field();
}


bool MultiStreamInput::next() {
    if (!streams_.empty()) {
        streams_.pop_front();
    }

    if (streams_.empty()) {
        return false;
    }

    return streams_.front()->next() || next();
}


bool MultiStreamInput::sameAs(const MIRInput& other) const {
    const auto* o = dynamic_cast<const MultiStreamInput*>(&other);

    if ((o == nullptr) || streams_.size() != o->streams_.size()) {
        return false;
    }

    for (auto s1 = streams_.cbegin(), s2 = o->streams_.cbegin(); s1 != streams_.cend(); ++s1, ++s2) {
        if (!(*s1)->sameAs(*(*s2))) {
            return false;
        }
    }

    return true;
}


void MultiStreamInput::print(std::ostream& out) const {
    out << "MultiStreamInput[";

    const auto* sep = "";
    for (const auto& s : streams_) {
        ASSERT(s != nullptr);
        out << sep << *s;
        sep = ",";
    }

    out << "]";
}


grib_handle* MultiStreamInput::gribHandle(size_t which) const {
    ASSERT(!streams_.empty());
    return streams_.front()->gribHandle(which);
}


}  // namespace input
}  // namespace mir
