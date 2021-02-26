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


#ifndef mir_key_packing_Packer_h
#define mir_key_packing_Packer_h

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
namespace key {
namespace packing {


class Packing {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Packing(const param::MIRParametrisation& user, const param::MIRParametrisation& field);
    Packing(const Packing&) = delete;

    // -- Destructor

    virtual ~Packing();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Packing&) = delete;

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

    friend std::ostream& operator<<(std::ostream& s, const Packing& p) {
        p.print(s);
        return s;
    }
};


class PackingFactory {
    std::string name_;
    virtual Packing* make(const param::MIRParametrisation& user, const param::MIRParametrisation& field) = 0;
    PackingFactory(const PackingFactory&)                                                                = delete;
    PackingFactory& operator=(const PackingFactory&) = delete;

protected:
    PackingFactory(const std::string&);
    virtual ~PackingFactory();

public:
    static Packing* build(const std::string&, const param::MIRParametrisation& user,
                          const param::MIRParametrisation& field);
    static void list(std::ostream&);
};


template <class T>
class PackingBuilder : public PackingFactory {
    Packing* make(const param::MIRParametrisation& user, const param::MIRParametrisation& field) override {
        return new T(user, field);
    }

public:
    PackingBuilder(const std::string& name) : PackingFactory(name) {}
};


}  // namespace packing
}  // namespace key
}  // namespace mir


#endif
