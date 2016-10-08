/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_RotatedGrid_h
#define mir_util_RotatedGrid_h

#include <cstddef>
#include <vector>
#include "eckit/memory/Builder.h"
#include "eckit/memory/ScopedPtr.h"
#include "atlas/grid/Domain.h"
#include "atlas/grid/Grid.h"


namespace mir {
namespace util {


class OffsetGrid : public atlas::grid::Grid {

public: // methods

    OffsetGrid(Grid *grid, double northwards, double eastwards);

    virtual ~OffsetGrid();

    virtual size_t npts() const;

    virtual void lonlat(std::vector<Point>&) const;

    virtual std::string gridType() const;

    virtual eckit::Properties spec() const;

private:  // methods

    /// Human readable name (may not be unique, especially when BoundBox is different)
    virtual std::string shortName() const;

    /// Adds to the MD5 the information that makes this Grid unique
    virtual void hash(eckit::MD5&) const;

    /// @return area represented by the grid
    virtual const atlas::grid::Domain& domain() const;

    virtual void print(std::ostream&) const;


    void computePoints() const;

private: // members


    eckit::ScopedPtr<Grid> grid_;

    double northwards_;
    double eastwards_;

    mutable std::string shortName_;
    mutable std::vector<Point> points_;

};


}  // namespace util
}  // namespace mir


#endif
