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

#include <iostream>
#include <sstream>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "mir/key/grid/GridPattern.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/regular/Lambert.h"
#include "mir/repres/regular/LambertAzimuthalEqualArea.h"


namespace mir {
namespace key {
namespace grid {


TypedGrid::TypedGrid(const std::string& key, const std::vector<std::string>& requiredKeys) :
    Grid(key, typed_t), requiredKeys_(requiredKeys) {}


void TypedGrid::print(std::ostream& out) const {
    out << "TypedGrid[key=" << key_ << ",requiredKeys=[";
    auto sep = "";
    for (auto& k : requiredKeys_) {
        out << sep << k;
        sep = ",";
    }
    out << "]]";
}


size_t TypedGrid::gaussianNumber() const {
    size_t N = 64;
    eckit::Log::warning() << "TypedGrid::gaussianNumber: setting N=" << N << " (hardcoded!)" << std::endl;
    return N;
}


template <typename Repres>
struct TypedGeneric final : public TypedGrid {
    using TypedGrid::TypedGrid;

    const repres::Representation* representation(const param::MIRParametrisation& param) const {
        // check for missing keys
        std::string missingKeys;
        auto sep = "";
        for (auto& key : requiredKeys_) {
            if (!param.has(key)) {
                missingKeys += sep + key;
                sep = ", ";
            }
        }

        if (!missingKeys.empty()) {
            std::ostringstream msg;
            msg << *this << ": expected keys are missing: " << missingKeys << std::endl;
            throw eckit::UserError(msg.str());
        }

        // return parametrised representation
        return new Repres(param);
    }
};


template <typename TYPE>
struct TypedGenericPattern final : public GridPattern {
    TypedGenericPattern(const std::string& name, const std::vector<std::string>& requiredKeys) :
        GridPattern(name), requiredKeys_(requiredKeys) {}

    TypedGenericPattern(const TypedGenericPattern&) = delete;
    TypedGenericPattern& operator=(const TypedGenericPattern&) = delete;

    const Grid* make(const std::string& name) const override {
        // register only the value of gridType= (which comes at the beggining)
        ASSERT(name.find("gridType=") == 0);
        auto key = name.substr(9);
        key      = key.substr(0, key.find(","));

        return new TYPE(key, requiredKeys_);
    }

    void print(std::ostream& out) const override {
        out << "TypedGenericPattern[pattern=" << pattern_ << ",requiredKeys=[";
        auto sep = "";
        for (auto& k : requiredKeys_) {
            out << sep << k;
            sep = ",";
        }
        out << "]]";
    }

    std::vector<std::string> requiredKeys_;
};


static TypedGenericPattern<TypedGeneric<repres::regular::Lambert>> __pattern1(
    "^gridType=lambert,.*$", {"LaDInDegrees", "LoVInDegrees", "Ni", "Nj", "grid", "latitudeOfFirstGridPointInDegrees",
                              "longitudeOfFirstGridPointInDegrees", "gaussianNumber"});


static TypedGenericPattern<TypedGeneric<repres::regular::LambertAzimuthalEqualArea>> __pattern2(
    "^gridType=lambert_azimuthal_equal_area,.*$",
    {"standardParallelInDegrees", "centralLongitudeInDegrees", "Ni", "Nj", "grid", "latitudeOfFirstGridPointInDegrees",
     "longitudeOfFirstGridPointInDegrees", "gaussianNumber"});


}  // namespace grid
}  // namespace key
}  // namespace mir
