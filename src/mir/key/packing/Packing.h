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


struct grib_handle;
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

    Packing(const std::string& name, const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Packing();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual void fill(const repres::Representation*, grib_info&) const  = 0;
    virtual void set(const repres::Representation*, grib_handle*) const = 0;

    virtual bool sameAs(Packing*) const;
    virtual bool printParametrisation(std::ostream&) const;

    virtual bool empty() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors

    Packing(const Packing&) = delete;

    // -- Operators

    void operator=(const Packing&) = delete;

    // -- Members

    long accuracy_;
    long edition_;
    std::string packing_;

    bool defineAccuracy_;
    bool defineEdition_;
    bool definePacking_;

    // -- Methods

    bool gridded() const { return gridded_; }
    void requireEdition(const param::MIRParametrisation&, long);

    void fill(grib_info&, long) const;
    void set(grib_handle*, const std::string&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const bool gridded_;

    // -- Methods
    // None
    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


class PackingFactory {
    std::string name_;
    bool spectral_;
    bool gridded_;

    virtual Packing* make(const std::string& name, const param::MIRParametrisation&) = 0;

    PackingFactory(const PackingFactory&) = delete;
    PackingFactory& operator=(const PackingFactory&) = delete;

protected:
    PackingFactory(const std::string&, const std::string&, bool spectral, bool gridded);
    virtual ~PackingFactory();

public:
    static Packing* build(const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class PackingBuilder : public PackingFactory {
    Packing* make(const std::string& name, const param::MIRParametrisation& param) override {
        return new T(name, param);
    }

public:
    PackingBuilder(const std::string& name, bool spectral, bool gridded) :
        PackingFactory(name, "", spectral, gridded) {}
    PackingBuilder(const std::string& name, const std::string& alias, bool spectral, bool gridded) :
        PackingFactory(name, alias, spectral, gridded) {}
};


}  // namespace packing
}  // namespace key
}  // namespace mir


#endif
