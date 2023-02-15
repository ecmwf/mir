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

#include "mir/action/plan/Action.h"


namespace mir::action {


class SelectField : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SelectField(const param::MIRParametrisation&);
    SelectField(const SelectField&) = delete;

    // -- Destructor

    ~SelectField() override;

    // -- Convertors
    // None

    // -- Operators

    SelectField& operator=(const SelectField&) = delete;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    void execute(context::Context&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    size_t which_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;
    const char* name() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action
