// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>

#include "mir/util/Types.h"


struct grib_info;
struct grib_handle;

namespace eckit {
class JSON;
}

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

    virtual void fill(const repres::Representation*, grib_info&) const = 0;

    /// Set the packing and the values, in the order ecCodes needs (as codes_grib_util_set_spec)
    virtual void set(const repres::Representation*, grib_handle*, const MIRValuesVector&) const = 0;

    bool sameAs(const Packing*) const;
    bool printParametrisation(std::ostream&) const;
    void fillEdition(grib_info&) const;
    void setEdition(grib_handle*) const;
    void json(eckit::JSON&) const;
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
    bool user_edition_ = false;
    bool definePacking_;
    bool definePrecision_;

    // -- Methods

    bool gridded() const { return gridded_; }
    void fill(grib_info&, long) const;
    void set(grib_handle*, const std::string& type, const MIRValuesVector&) const;

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
