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

    Packer(const param::MIRParametrisation& user, const param::MIRParametrisation& field);
    Packer(const Packer&) = delete;

    // -- Destructor

    virtual ~Packer();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Packer&) = delete;

    // -- Methods

    virtual void fill(grib_info&, const repres::Representation&) const = 0;
    virtual std::string type(const repres::Representation*) const      = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors
    // None

    // -- Members
    // None

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


class PackerFactory {
    std::string name_;
    virtual Packer* make(const param::MIRParametrisation& user, const param::MIRParametrisation& field) = 0;
    PackerFactory(const PackerFactory&)                                                                 = delete;
    PackerFactory& operator=(const PackerFactory&) = delete;

protected:
    PackerFactory(const std::string&);
    virtual ~PackerFactory();

public:
    static Packer* build(const std::string&, const param::MIRParametrisation& user,
                         const param::MIRParametrisation& field);
    static void list(std::ostream&);
};


template <class T>
class PackerBuilder : public PackerFactory {
    virtual Packer* make(const param::MIRParametrisation& user, const param::MIRParametrisation& field) {
        return new T(user, field);
    }

public:
    PackerBuilder(const std::string& name) : PackerFactory(name) {}
};


}  // namespace packing
}  // namespace mir


#endif
