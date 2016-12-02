/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef MIRWatcher_H
#define MIRWatcher_H


#include "eckit/memory/NonCopyable.h"

namespace mir {

namespace action {
class Action;
}

namespace api {

class MIRJob;

class MIRWatcher : private eckit::NonCopyable {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    MIRWatcher();

    // -- Destructor

    virtual ~MIRWatcher();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // Return true to rethrow, false to continue
    virtual bool failure(std::exception&, const action::Action& action) = 0;

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



    // -- Overridden methods


    // -- Class members
    // None

    // -- Class methods
    // None

  private:


    // -- Members


    // -- Methods


    // -- Overridden methods

    // From MIRParametrisation

    virtual void print(std::ostream &) const = 0;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const MIRWatcher &p) {
        p.print(s);
        return s;
    }
};


}  // namespace api
}  // namespace mir
#endif

