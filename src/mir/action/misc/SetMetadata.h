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

#include <map>

#include "mir/action/plan/Action.h"


namespace mir {
namespace action {


class SetMetadata : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SetMetadata(const param::MIRParametrisation&);
    SetMetadata(const SetMetadata&) = delete;

    // -- Destructor

    ~SetMetadata() override;

    // -- Convertors
    // None

    // -- Operators

    SetMetadata& operator=(const SetMetadata&) = delete;

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

    std::map<std::string, long> metadata_;

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
    // None

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


}  // namespace action
}  // namespace mir
