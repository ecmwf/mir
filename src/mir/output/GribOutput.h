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


#ifndef GribOutput_H
#define GribOutput_H

#include "mir/output/MIROutput.h"

struct grib_info;
struct grib_handle;

namespace eckit {
class DataHandle;
}


namespace mir {


namespace output {


class GribOutput : public MIROutput {
public:

// -- Exceptions
    // None

// -- Contructors

    GribOutput();

// -- Destructor

    ~GribOutput(); // Change to virtual if base class

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
    // None

protected:

// -- Members
    // None

// -- Methods

    virtual void fill(grib_handle* handle, grib_info& info) const;

// -- Overridden methods
    // From MIROutput

    virtual size_t copy(const param::MIRParametrisation &, context::Context &); // Not iterpolation performed
    virtual size_t save(const param::MIRParametrisation&, context::Context&);
    virtual bool sameParametrisation(const param::MIRParametrisation &, const param::MIRParametrisation &) const;
    virtual bool printParametrisation(std::ostream& out, const param::MIRParametrisation &param) const;

// -- Class members
    // None

// -- Class methods
    // None

private:

// -- Members

    unsigned long long total_;

// -- Methods

    virtual void out(const void* message, size_t length, bool iterpolated) = 0;

// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const GribOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

