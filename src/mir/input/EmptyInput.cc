// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/EmptyInput.h"

#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir::input {


EmptyInput::EmptyInput() : calls_(0) {
    parametrisation_.set("gridded", true);
    parametrisation_.set("gridType", "none");
}


EmptyInput::~EmptyInput() = default;


bool EmptyInput::sameAs(const MIRInput& other) const {
    const auto* o = dynamic_cast<const EmptyInput*>(&other);
    return (o != nullptr);
}


bool EmptyInput::next() {
    return calls_++ == 0;
}


const param::MIRParametrisation& EmptyInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return parametrisation_;
}


data::MIRField EmptyInput::field() const {
    return data::MIRField(parametrisation_, false, 999.);
}


void EmptyInput::print(std::ostream& out) const {
    out << "EmptyInput[...]";
}


void EmptyInput::latitudes(std::vector<double>& /*unused*/) const {}


void EmptyInput::longitudes(std::vector<double>& /*unused*/) const {}


}  // namespace mir::input
