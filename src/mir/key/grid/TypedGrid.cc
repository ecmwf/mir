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


#include "mir/key/grid/TypedGrid.h"

#include <ostream>
#include <sstream>

#include "eckit/utils/StringTools.h"

#include "mir/key/grid/GridPattern.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/regular/Lambert.h"
#include "mir/repres/regular/LambertAzimuthalEqualArea.h"
#include "mir/repres/regular/PolarStereographic.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::key::grid {


TypedGrid::TypedGrid(const std::string& key, const std::set<std::string>& requiredKeys,
                     const std::set<std::string>& optionalKeys) :
    Grid(key, "typedgrid"), requiredKeys_(requiredKeys), optionalKeys_(optionalKeys) {
    requiredKeys_.insert("gridType");
}


void TypedGrid::print(std::ostream& out) const {
    out << "TypedGrid[key=" << key_ << ",requiredKeys=[";
    const auto* sep = "";
    for (const auto& k : requiredKeys_) {
        out << sep << k;
        sep = ",";
    }
    out << "]";
    out << ",optionalKeys=[";
    sep = "";
    for (const auto& k : optionalKeys_) {
        out << sep << k;
        sep = ",";
    }
    out << "]]";
}


void TypedGrid::parametrisation(const std::string& grid, param::SimpleParametrisation& param) const {
    // set a new parametrisation containing only required or optional keys
    param::SimpleParametrisation p;
    for (auto& kv_str : eckit::StringTools::split(";", grid)) {
        if (auto it = kv_str.find("="); it != std::string::npos) {
            if (auto k = kv_str.substr(0, it), v = kv_str.substr(it + 1); !k.empty() && !v.empty()) {
                if (requiredKeys_.find(k) != requiredKeys_.end() || optionalKeys_.find(k) != optionalKeys_.end()) {
                    p.set(k, v);
                    continue;
                }
            }
        }

        throw exception::UserError("TypedGrid: invalid key=value pair, got '" + kv_str + "'");
    }

    // check for missing keys, set return parametrisation
    checkRequiredKeys(p);
    param.swap(p);
}


size_t TypedGrid::gaussianNumber() const {
    // FIXME: Resol (deprecated!!!) gets gaussianNumber without instantiating a Representation
    param::SimpleParametrisation param;
    parametrisation(key_, param);

    long N = 0;
    return param.get("gaussianNumber", N) && N > 0 ? size_t(N) : default_gaussian_number();
}


void TypedGrid::checkRequiredKeys(const param::MIRParametrisation& param) const {
    std::string missingKeys;

    const auto* sep = "";
    for (const auto& key : requiredKeys_) {
        if (!param.has(key)) {
            missingKeys += sep + key;
            sep = ", ";
        }
    }

    if (!missingKeys.empty()) {
        std::ostringstream msg;
        msg << *this << ": required keys are missing: " << missingKeys;
        Log::error() << msg.str() << std::endl;
        throw exception::UserError(msg.str());
    }
}


template <typename Repres>
struct TypedGeneric final : public TypedGrid {
    using TypedGrid::TypedGrid;

    const repres::Representation* representation(const param::MIRParametrisation& param) const override {
        // check for missing keys, set return representation
        checkRequiredKeys(param);
        return new Repres(param);
    }
};


template <typename TYPE>
struct TypedGenericPattern final : public GridPattern {
    TypedGenericPattern(const std::string& pattern, const std::set<std::string>& requiredKeys,
                        const std::set<std::string>& optionalKeys = {}) :
        GridPattern(pattern), requiredKeys_(requiredKeys), optionalKeys_(optionalKeys) {}

    const Grid* make(const std::string& name) const override { return new TYPE(name, requiredKeys_, optionalKeys_); }

    std::string canonical(const std::string& name, const param::MIRParametrisation& /*unused*/) const override {
        // FIXME not implemented
        return name;
    }

    void print(std::ostream& out) const override {
        out << "TypedGenericPattern[pattern=" << pattern_ << ",requiredKeys=[";
        const auto* sep = "";
        for (const auto& k : requiredKeys_) {
            out << sep << k;
            sep = ",";
        }
        out << "]";
        out << ",optionalKeys=[";
        sep = "";
        for (const auto& k : optionalKeys_) {
            out << sep << k;
            sep = ",";
        }
        out << "]]";
    }

private:
    std::set<std::string> requiredKeys_;
    std::set<std::string> optionalKeys_;
};


static const TypedGenericPattern<TypedGeneric<repres::regular::Lambert>> __pattern1(
    "^gridType=lambert;.*$",
    {"LaDInDegrees", "LoVInDegrees", "Ni", "Nj", "grid", "latitudeOfFirstGridPointInDegrees",
     "longitudeOfFirstGridPointInDegrees"},
    {"Latin1InDegrees", "Latin2InDegrees", "writeLaDInDegrees", "writeLonPositive", "gaussianNumber", "shapeOfTheEarth",
     "radius", "earthMajorAxis", "earthMinorAxis"});


static const TypedGenericPattern<TypedGeneric<repres::regular::LambertAzimuthalEqualArea>> __pattern2(
    "^gridType=lambert_azimuthal_equal_area;.*$",
    {"standardParallelInDegrees", "centralLongitudeInDegrees", "Ni", "Nj", "grid", "latitudeOfFirstGridPointInDegrees",
     "longitudeOfFirstGridPointInDegrees"},
    {"gaussianNumber", "shapeOfTheEarth", "radius", "earthMajorAxis", "earthMinorAxis"});


static const TypedGenericPattern<TypedGeneric<repres::regular::PolarStereographic>> __pattern3(
    "^gridType=polar_stereographic;.*$",
    {"proj", "LaDInDegrees", "orientationOfTheGridInDegrees", "southPoleOnProjectionPlane", "Ni", "Nj", "grid",
     "latitudeOfFirstGridPointInDegrees", "longitudeOfFirstGridPointInDegrees"},
    {"gaussianNumber", "shapeOfTheEarth", "radius", "earthMajorAxis", "earthMinorAxis", "iScansNegatively",
     "jScansPositively"});


}  // namespace mir::key::grid
