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


#include "mir/input/ConstantInput.h"

#include <iostream>

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace input {


static ArtificialInputBuilder<ConstantInput> __artificial("constant");


ConstantInput::ConstantInput(const param::MIRParametrisation& parametrisation) : ArtificialInput(parametrisation) {
    parametrisation.get("constant", constant_ = 0.);
}


bool ConstantInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const ConstantInput*>(&other);
    return (o != nullptr) && constant_ == constant_ && ArtificialInput::sameAs(other);
}


void ConstantInput::print(std::ostream& out) const {
    out << "ConstantInput[constant=" << constant_ << ",";
    ArtificialInput::print(out);
    out << "]";
}


MIRValuesVector ConstantInput::fill(size_t n) const {
    MIRValuesVector values(n, constant_);
    return values;
}


}  // namespace input
}  // namespace mir
