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


#pragma once

#include "mir/input/ArtificialInput.h"


namespace mir {
namespace input {


class RawInput final : public ArtificialInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RawInput(const double* values, size_t count, const param::SimpleParametrisation& metadata);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const double* values_;
    size_t count_;

    // -- Methods

    // -- Overridden methods

    // From MIRInput
    data::MIRField field() const override;

    // From ArtificialInput
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace input
}  // namespace mir
