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
#include "mir/util/Grib.h"


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::util {


struct Shape {
    using Projection = ::atlas::Projection;

    Shape(const param::MIRParametrisation&);
    Shape(const Projection::Spec&);
    Shape() : code(6L), a(0.), b(0.), provided(false) {}
    Shape(const Shape& other);
    virtual ~Shape() = default;

    Shape& operator=(const Shape& other);
    void fillGrib(grib_info& info, const Projection::Spec&) const;

    long code;
    double a;
    double b;
    long edition;
    bool provided;
};


}  // namespace mir::util
