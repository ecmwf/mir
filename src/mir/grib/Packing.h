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


#pragma once

#include <iosfwd>
#include <string>


struct grib_info;
struct grib_handle;

namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::grib {


class Packing {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    Packing(const std::string& name, const param::MIRParametrisation&);

    Packing(const Packing&) = delete;
    Packing(Packing&&)      = delete;

    // -- Destructor

    virtual ~Packing() = default;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Packing&) = delete;
    void operator=(Packing&&)      = delete;

    // -- Methods

    virtual void fill(const repres::Representation*, grib_info&) const  = 0;
    virtual void set(const repres::Representation*, grib_handle*) const = 0;

    bool sameAs(const Packing*) const;
    bool printParametrisation(std::ostream&) const;
    bool empty() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static Packing* build(const param::MIRParametrisation&);
    static void list(std::ostream&);

protected:
    // -- Members

    long bitsPerValue_;
    long edition_;
    std::string packing_;
    long precision_;

    bool defineBitsPerValue_;
    bool defineBitsPerValueBeforePacking_;
    bool defineEdition_;
    bool definePacking_;
    bool definePrecision_;

    // -- Methods

    bool gridded() const { return gridded_; }
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


}  // namespace mir::grib
