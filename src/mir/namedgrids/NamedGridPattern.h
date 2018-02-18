/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef mir_namedgrids_NamedGridPattern_h
#define mir_namedgrids_NamedGridPattern_h

#include <iosfwd>
#include <string>

#include "eckit/utils/Regex.h"
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace namedgrids {

class NamedGrid;

class NamedGridPattern : private eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors
    // None

    // -- Destructor
    // None

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

    static bool match(const std::string &name);
    static const NamedGrid* build(const std::string &name);
    static void list(std::ostream &);

protected:

    NamedGridPattern(const std::string &pattern);
    virtual ~NamedGridPattern();

    // -- Members

    eckit::Regex pattern_;

    // -- Methods

    virtual const NamedGrid* make(const std::string &name) const = 0;
    virtual void print(std::ostream &) const = 0;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const NamedGridPattern &p) {
        p.print(s);
        return s;
    }

};


}  // namespace namedgrids
}  // namespace mir


#endif

