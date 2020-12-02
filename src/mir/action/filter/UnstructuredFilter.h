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


#ifndef mir_action_UnstructuredFilter_h
#define mir_action_UnstructuredFilter_h

#include "mir/action/plan/Action.h"


namespace mir {
namespace action {


class UnstructuredFilter : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    using Action::Action;

    // -- Destructor

    virtual ~UnstructuredFilter() override;

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

    void print(std::ostream&) const;

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

    virtual void execute(context::Context&) const;
    virtual bool sameAs(const Action&) const;
    virtual const char* name() const;
    virtual void estimate(context::Context&, api::MIREstimation&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace action
}  // namespace mir


#endif
