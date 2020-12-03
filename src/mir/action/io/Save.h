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


#ifndef mir_action_io_Save_h
#define mir_action_io_Save_h

#include "mir/action/plan/Action.h"


namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
}  // namespace mir


namespace mir {
namespace action {
namespace io {


class Save : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Save(const param::MIRParametrisation&, input::MIRInput&, output::MIROutput&);

    // -- Destructor

    virtual ~Save() override;  // Change to virtual if base class

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

    void print(std::ostream&) const;  // Change to virtual if base class
    void custom(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // No copy allowed

    Save(const Save&);
    Save& operator=(const Save&);

    // -- Members

    input::MIRInput& input_;
    output::MIROutput& output_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void execute(context::Context&) const override;
    virtual bool sameAs(const Action&) const override;
    virtual const char* name() const;
    virtual bool isEndAction() const;
    virtual void estimate(context::Context&, api::MIREstimation& estimation) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const Save& p)
    //  { p.print(s); return s; }
};


}  // namespace io
}  // namespace action
}  // namespace mir


#endif
