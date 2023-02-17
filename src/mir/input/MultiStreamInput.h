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

#include <deque>

#include "mir/input/MIRInput.h"


namespace mir::input {


class MultiStreamInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors
    MultiStreamInput();

    // -- Destructor

    ~MultiStreamInput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void append(MIRInput*);

    // -- Overridden methods

    size_t dimensions() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
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

    std::deque<MIRInput*> streams_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRInput
    const param::MIRParametrisation& parametrisation(size_t which) const override;
    data::MIRField field() const override;
    bool next() override;
    bool sameAs(const MIRInput&) const override;
    void print(std::ostream&) const override;
    grib_handle* gribHandle(size_t which = 0) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::input
