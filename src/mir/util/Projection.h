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

// #include <iosfwd>
#include <memory>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/geo/Projection.h"
// #include "eckit/utils/Hash.h"

#include "mir/util/Types.h"


// struct grib_info;

// namespace atlas {
// class Grid;
// class Projection;
// }  // namespace atlas

namespace mir {
// namespace api {
// class MIRJob;
// }
// namespace param {
// class MIRParametrisation;
// }
namespace util {
class BoundingBox;
}
}  // namespace mir


namespace mir::util {


struct Config : public eckit::LocalConfiguration {
    Config() : LocalConfiguration('.') {}
    Config(const std::string&, const std::string&) : LocalConfiguration('.') {}
    template <typename T>
    Config& set(const std::string& a, const T& b) {
        LocalConfiguration::set(a, b);
        return *this;
    }
};


class Projection {
public:
    using Spec = Config;
    Spec spec_;
    Spec spec() const;
    void hash(eckit::Hash& h) const;

    Projection() = default;
    Projection(const Spec&);
    explicit operator bool() const;
    Point2 xy(const Point2& p) const;
    PointLonLat lonlat(const Point2& p) const;

    BoundingBox lonlatBoundingBox(Point2 min, Point2 max) const;

private:
    std::unique_ptr<eckit::geo::Projection> proj_;
};


}  // namespace mir::util
