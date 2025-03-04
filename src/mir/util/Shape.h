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

#include "mir/util/Atlas.h"


struct grib_info;

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

    long code;
    double a;
    double b;
    bool provided;
};


}  // namespace mir::util
