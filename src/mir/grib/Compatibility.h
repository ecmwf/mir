// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>


namespace mir {
namespace grib {
class OutputKeys;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::grib {


/// GRIB output compatibilities of production (pgen), as compatibility=<name>[/<name>...]:
/// - no-local-extension: GRIB1 without local definition
/// - adjust-spectral-packing: spectral simple packing (as requested) as complex
class Compatibility {
public:
    // -- Constructors

    /// Of the user parametrisation, throwing on an unknown compatibility
    explicit Compatibility(const param::MIRParametrisation&);

    // -- Methods

    bool empty() const { return name_.empty(); }
    const std::string& name() const { return name_; }

    /// Add the keys set after the geometry, of the edition encoded
    void keys(long edition, OutputKeys&) const;

    /// The packing type as requested (by name, or as codes_grib_util_set_spec), adjusted
    std::string packing_type(const std::string&) const;
    long packing_type(long) const;

    // -- Class methods

    static void list(std::ostream&);

private:
    // -- Members

    std::string name_;
    bool noLocalExtension_      = false;
    bool adjustSpectralPacking_ = false;
};


}  // namespace mir::grib
