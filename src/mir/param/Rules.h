// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <map>
#include <set>
#include <string>

#include "mir/util/Mutex.h"


namespace mir::param {
class MIRParametrisation;
class SimpleParametrisation;
}  // namespace mir::param


namespace mir::param {


class Rules {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    Rules();

    // -- Destructor

    ~Rules();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const MIRParametrisation* find(const param::MIRParametrisation&) const;

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

    util::recursive_mutex mutex_;
    std::map<long, SimpleParametrisation*> rules_;
    std::set<long> noted_;
    std::set<long> warning_;

    // -- Methods

    const MIRParametrisation& lookup(const std::string& ruleName, long ruleValue);
    SimpleParametrisation& lookup(long paramId);

    void readConfigurationFiles();
    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Rules& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::param
