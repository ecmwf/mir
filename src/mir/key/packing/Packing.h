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

    Packing(const param::MIRParametrisation&, bool gridded);
    Packing(const Packing&) = delete;

    // -- Destructor

    virtual ~Packing();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Packing&) = delete;

    // -- Methods

    virtual void fill(grib_info&) const  = 0;
    virtual void set(grib_handle*) const = 0;

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

    bool userPacking() const { return userPacking_; }
    bool gridded() const { return gridded_; }

    void saveAccuracy(grib_info&) const;
    void saveEdition(grib_info&) const;
    void savePacking(grib_info&, long) const;
    void setAccuracy(grib_handle*) const;
    void setEdition(grib_handle*) const;
    void setPacking(grib_handle*, const std::string&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string packing_;
    long accuracy_;
    long edition_;

    const bool userPacking_;
    const bool userAccuracy_;
    const bool userEdition_;

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

    friend std::ostream& operator<<(std::ostream& s, const Packing& p) {
        p.print(s);
        return s;
    }
};


class PackingFactory {
    std::string name_;
    std::string alias_;
    bool spectral_;
    bool gridded_;

    virtual Packing* make(const param::MIRParametrisation&, bool gridded) = 0;

    PackingFactory(const PackingFactory&) = delete;
    PackingFactory& operator=(const PackingFactory&) = delete;

protected:
    PackingFactory(const std::string&, const std::string&, bool spectral, bool gridded);
    virtual ~PackingFactory();

public:
    static Packing* build(const std::string&, const param::MIRParametrisation&, const repres::Representation*);
    static Packing* build(const param::MIRParametrisation&, const repres::Representation*);
    static void list(std::ostream&);
    static bool get(std::string&, const param::MIRParametrisation&);
};


template <class T>
class PackingBuilder : public PackingFactory {
    Packing* make(const param::MIRParametrisation& param, bool gridded) override { return new T(param, gridded); }

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
