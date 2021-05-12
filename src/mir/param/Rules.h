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
#include <set>
#include <string>

#include "mir/util/Mutex.h"


namespace mir {
namespace param {
class MIRParametrisation;
class SimpleParametrisation;
}  // namespace param
}  // namespace mir


namespace mir {
namespace param {


class Rules {
public:
    // -- Operators

    static const MIRParametrisation& lookup(const std::string& ruleName, long ruleValue);

private:
    // -- Constructors

    Rules();

    // -- Destructor

    virtual ~Rules();

    // -- Members

    util::recursive_mutex mutex_;
    std::map<long, SimpleParametrisation*> rules_;
    std::set<long> noted_;
    std::set<long> warning_;

    // -- Methods

    static Rules& instance() {
        static Rules fileRules;
        return fileRules;
    }

    void readConfigurationFiles();
    SimpleParametrisation& lookup(long paramId);
    virtual void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Rules& p) {
        p.print(s);
        return s;
    }
};


}  // namespace param
}  // namespace mir
