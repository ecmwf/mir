// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <exception>
#include <iosfwd>


namespace mir::action {
class Action;
}  // namespace mir::action


namespace mir::api {


class MIRWatcher {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MIRWatcher();

    MIRWatcher(const MIRWatcher&) = delete;
    MIRWatcher(MIRWatcher&&)      = delete;

    // -- Destructor

    virtual ~MIRWatcher();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const MIRWatcher&) = delete;
    void operator=(MIRWatcher&&)      = delete;

    // -- Methods

    // Return true to rethrow, false to continue
    virtual bool failure(std::exception&, const action::Action&) = 0;

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
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRParametrisation
    virtual void print(std::ostream&) const = 0;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MIRWatcher& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::api
