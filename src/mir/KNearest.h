/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef mir_KNearest_H
#define mir_KNearest_H

#include <Eigen/Sparse>

#include "mir/Weights.h"

//-----------------------------------------------------------------------------

namespace atlas {
namespace grid {
    class Point2;
}
}

namespace mir {

//-----------------------------------------------------------------------------

class KNearest: public Weights {

public:

    KNearest();
    virtual ~KNearest();

    virtual void compute( Grid& in, Grid& out, Eigen::SparseMatrix<double>& W ) const;

    virtual std::string classname() const;

private:

    size_t nclosest_; ///< number of closest points to search for

};

//-----------------------------------------------------------------------------

} // namespace mir

#endif
