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


#ifndef mir_packing_Packer_h
#define mir_packing_Packer_h

#include <iosfwd>
#include <string>


struct grib_info;

namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir {
namespace packing {


class Packer {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Packer(const Packer&) = delete;

    // -- Destructor

    virtual ~Packer();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Packer&) = delete;

    // -- Methods

    static const Packer& lookup(const std::string&);
    static void list(std::ostream&);
    virtual void fill(grib_info&, const repres::Representation&, const param::MIRParametrisation& user,
                      const param::MIRParametrisation& field) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors

    Packer(const std::string& name);

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

    friend std::ostream& operator<<(std::ostream& s, const Packer& p) {
        p.print(s);
        return s;
    }
};


}  // namespace packing
}  // namespace mir


#endif
