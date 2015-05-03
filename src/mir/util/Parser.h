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


#ifndef Parser_H
#define Parser_H


#include "eckit/filesystem/PathName.h"
#include "eckit/parser/StreamParser.h"
#include <fstream>
#include "mir/util/ParserConsumer.h"

namespace mir {
namespace util {


class Parser  {
  public:

// -- Exceptions
    // None

// -- Contructors

    Parser(const eckit::PathName&);

// -- Destructor

    ~Parser(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    void fill(ParserConsumer&);

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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    Parser(const Parser&);
    Parser& operator=(const Parser&);

// -- Members

    eckit::PathName path_;
    std::ifstream in_;
    eckit::StreamParser parser_;

// -- Methods

    void consumeComment();
    bool readNumber(long& lvalue, double& dvalue);

    char peek();
    char next();
// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Parser& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir
#endif

