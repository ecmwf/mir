/*
 * (C) Copyright 1996-2014 ECMWF.
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

#ifndef mir_Weights_H
#define mir_Weights_H

#include "eckit/memory/NonCopyable.h"

//-----------------------------------------------------------------------------

namespace eckit {
namespace grid {
    class Point2D;
}
}


namespace mir {

//-----------------------------------------------------------------------------

class WeightEngine : private eckit::NonCopyable {
public:
    WeightEngine() {};
    virtual ~WeightEngine() {};

    virtual void generate(const eckit::grid::Point2D& ref, const std::vector<eckit::grid::Point2D>& closests, std::vector<double>& weights) const = 0;

};


class InverseSquare: public WeightEngine {

public:
    InverseSquare();
    virtual ~InverseSquare();

    virtual void generate(const eckit::grid::Point2D& ref, const std::vector<eckit::grid::Point2D>& closests, std::vector<double>& weights) const;
protected:
};


} // namespace mir

#endif
