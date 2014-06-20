/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef mir_Bilinear_H
#define mir_Bilinear_H

#include "mir/Weights.h"

//-----------------------------------------------------------------------------

namespace atlas {
namespace grid {
    class Point2;
}
}

namespace eckit {
namespace geometry {
    class Point3;
}
}


namespace mir {

//-----------------------------------------------------------------------------

class Bilinear: public Weights {

public:

    Bilinear();
    virtual ~Bilinear();

    virtual void compute( Grid& in, Grid& out, Weights::Matrix& W ) const;

    virtual std::string classname() const;

private:

    size_t nclosest_; ///< number of closest points to search for

    double crossProduct(const eckit::geometry::Point3& a, const eckit::geometry::Point3& b) const;
    bool formClockwiseTriangle(const eckit::geometry::Point3& a, const eckit::geometry::Point3& b, const eckit::geometry::Point3& c) const;
    void sort4Clockwise(std::vector<eckit::geometry::Point3>& points) const ;


};

//-----------------------------------------------------------------------------

} // namespace mir

#endif
