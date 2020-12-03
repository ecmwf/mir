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


#ifndef mir_action_plan_SimpleExecutor_h
#define mir_action_plan_SimpleExecutor_h

#include <iosfwd>

#include "mir/action/plan/Executor.h"


namespace mir {
namespace action {


class SimpleExecutor : public Executor {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SimpleExecutor(const std::string& name);

    // -- Destructor

    ~SimpleExecutor();  // Change to virtual if base class

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

    void print(std::ostream&) const override;  // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // No copy allowed
    SimpleExecutor(const SimpleExecutor&);
    SimpleExecutor& operator=(const SimpleExecutor&);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    //
    virtual void execute(context::Context&, const ActionNode&) const override;
    virtual void wait() const override;
    virtual void parametrisation(const param::MIRParametrisation&) override;

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
