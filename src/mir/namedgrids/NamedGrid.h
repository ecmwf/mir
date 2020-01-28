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


#ifndef mir_namedgrids_NamedGrid_h
#define mir_namedgrids_NamedGrid_h

#include <iosfwd>
#include <string>


namespace mir {
namespace repres {
class Representation;
}
namespace util {
class Rotation;
}
}  // namespace mir


namespace mir {
namespace namedgrids {

class NamedGrid {
public:
    // -- Exceptions
    // None

    // -- Contructors

    NamedGrid(const NamedGrid&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    NamedGrid& operator=(const NamedGrid&) = delete;

    // -- Methods

    virtual const repres::Representation* representation() const                               = 0;
    virtual const repres::Representation* representation(const util::Rotation& rotation) const = 0;
    virtual size_t gaussianNumber() const                                                      = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const NamedGrid& lookup(const std::string& name);
    static void list(std::ostream&);


protected:
    NamedGrid(const std::string& name);
    virtual ~NamedGrid();

    // -- Members

    std::string name_;

    // -- Methods

    virtual void print(std::ostream&) const = 0;

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

    friend std::ostream& operator<<(std::ostream& s, const NamedGrid& p) {
        p.print(s);
        return s;
    }
};


}  // namespace namedgrids
}  // namespace mir


#endif
