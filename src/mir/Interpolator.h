/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @date Oct 2013

#ifndef eckit_grid_Interpolator_H
#define eckit_grid_Interpolator_H

#include "eckit/memory/NonCopyable.h"

//-----------------------------------------------------------------------------

namespace eckit {
namespace grid {
    class FieldSet;
    class Point2D;
}
}


namespace mir {

//-----------------------------------------------------------------------------

class Interpolator : private eckit::NonCopyable {

public: // methods

    Interpolator();

    virtual ~Interpolator();

    virtual void interpolate(const eckit::grid::FieldSet& input, eckit::grid::FieldSet& output) = 0;

protected:
    

};

//-----------------------------------------------------------------------------

class Bilinear : public Interpolator {

public:
    Bilinear();
    virtual ~Bilinear();

    virtual void interpolate(const eckit::grid::FieldSet& input, eckit::grid::FieldSet& output);
protected:
    double unnormalisedWeight(const eckit::grid::Point2D& p, const eckit::grid::Point2D& o) const;
};

} // namespace mir

#endif
