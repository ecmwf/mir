// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Atlas.h"


struct grib_info;

namespace eckit::spec {
class Custom;
}  // namespace eckit::spec

namespace mir {
namespace param {
class MIRParametrisation;
}
namespace api {
class MIRJob;
}
}  // namespace mir


namespace mir::util {


struct Shape {
    using Projection = ::atlas::Projection;

    Shape(const param::MIRParametrisation&);
    Shape(const Projection::Spec&);
    Shape() : code(6L), a(0.), b(0.), provided(false) {}

    Shape(const Shape&) = default;
    Shape(Shape&&)      = default;

    virtual ~Shape() = default;

    Shape& operator=(const Shape&) = default;
    Shape& operator=(Shape&&)      = default;

    void fillGrib(grib_info&, const Projection::Spec&) const;
    void fillJob(api::MIRJob&, const Projection::Spec&) const;
    void fillSpec(eckit::spec::Custom&, const Projection::Spec&) const;

    long code;
    double a;
    double b;
    bool provided;
};


}  // namespace mir::util
