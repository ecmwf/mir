/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_param_rules_Rules_h
#define mir_param_rules_Rules_h

#include <map>
#include "eckit/memory/Owned.h"
#include "eckit/memory/SharedPtr.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace param {
namespace rules {


// provide attach/detach to SimpleParametrisation under SharedPtr
struct CountedParametrisation : SimpleParametrisation, eckit::OwnedLock {};


class Rules : protected std::map< long, eckit::SharedPtr<CountedParametrisation> > {
protected:

    // -- Types

    typedef std::map< long, eckit::SharedPtr<CountedParametrisation> > container_t;

public:

    // -- Contructors

    Rules();

    // -- Destructor

    virtual ~Rules();

    // -- Operators

    const MIRParametrisation& operator[](const long& paramId) const;

    // -- Methods

    SimpleParametrisation& modify(const long& paramId);

private:

    // -- Methods

    virtual void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Rules& p) {
        p.print(s);
        return s;
    }

};


}  // namespace rules
}  // namespace param
}  // namespace mir


#endif

