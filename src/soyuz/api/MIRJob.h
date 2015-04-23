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


#ifndef MIRJob_H
#define MIRJob_H

#include <map>
#include <string>

#include "eckit/memory/NonCopyable.h"

#include "soyuz/param/MIRParametrisation.h"


namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace api {


class MIRJob : public param::MIRParametrisation, public eckit::NonCopyable {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIRJob();

// -- Destructor

    virtual ~MIRJob();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    void execute(input::MIRInput&, output::MIROutput&) const;

    void set(const std::string&, const std::string&);

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

    std::map<std::string, std::string> settings_;

// -- Methods

    bool matches(const param::MIRParametrisation&) const;

// -- Overridden methods

    // From MIRParametrisation

    virtual void print(std::ostream&) const;
    virtual bool get(const std::string&, std::string&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

};


}  // namespace api
}  // namespace mir
#endif

