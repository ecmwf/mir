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


class EmptyInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors
    EmptyInput();

    // -- Destructor

    ~EmptyInput() override;

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

    size_t calls_;
    param::SimpleParametrisation parametrisation_;

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;
    bool sameAs(const MIRInput&) const override;

    const param::MIRParametrisation& parametrisation(size_t which) const override;
    data::MIRField field() const override;

    bool next() override;

    // bool get(const std::string&, double&) const override;

    virtual void latitudes(std::vector<double>&) const;
    virtual void longitudes(std::vector<double>&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::input
