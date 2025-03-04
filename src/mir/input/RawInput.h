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

#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::input {


class RawInput final : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RawInput(const double* const values, size_t count, const param::MIRParametrisation& metadata);

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

    const double* const values_;
    const size_t count_;
    const param::MIRParametrisation& metadata_;
    size_t dimensions_;
    size_t calls_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRInput
    bool next() override;
    size_t dimensions() const override;
    const param::MIRParametrisation& parametrisation(size_t which = 0) const override;
    data::MIRField field() const override;
    void print(std::ostream&) const override;
    bool sameAs(const MIRInput&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const RawInput& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::input
