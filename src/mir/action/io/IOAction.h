/*
 * (C) EndActionright 1996- ECMWF.
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


namespace mir {
namespace output {
class MIROutput;
}
}  // namespace mir


namespace mir {
namespace action {
namespace io {


class IOAction : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    IOAction(const param::MIRParametrisation&, output::MIROutput&);
    IOAction(const IOAction&) = delete;

    // -- Destructor

    virtual ~IOAction() override;

    // -- Convertors
    // None

    // -- Operators

    IOAction& operator=(const IOAction&) = delete;

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

    output::MIROutput& output() { return output_; }
    const output::MIROutput& output() const { return output_; }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    output::MIROutput& output_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;
    bool isEndAction() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace io
}  // namespace action
}  // namespace mir
