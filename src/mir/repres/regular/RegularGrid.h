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


#ifndef mir_repres_regular_RegularGrid_h
#define mir_repres_regular_RegularGrid_h

#include "mir/api/Atlas.h"

#include "mir/repres/Gridded.h"


namespace mir {
namespace repres {
namespace regular {

class RegularGrid : public Gridded {
public:
    // -- Types

    using LinearSpacing = ::atlas::grid::LinearSpacing;
    using PointLonLat   = ::atlas::PointLonLat;
    using Projection    = ::atlas::Projection;

    // -- Exceptions
    // None

    // -- Contructors

    RegularGrid(const param::MIRParametrisation&, const Projection&);
    RegularGrid(const RegularGrid&) = delete;
    RegularGrid& operator=(const RegularGrid&) = delete;

    // -- Destructor

    virtual ~RegularGrid();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Types

    using XYZCOORDS = eckit::geometry::XYZCOORDS;
    using LLCOORDS  = eckit::geometry::LLCOORDS;

    // -- Members

    ::atlas::RegularGrid grid_;
    LinearSpacing x_;
    LinearSpacing y_;

    // -- Methods
    // None

    // -- Overridden methods

    // from Representation
    virtual bool sameAs(const Representation&) const;

    virtual bool includesNorthPole() const;
    virtual bool includesSouthPole() const;
    virtual bool isPeriodicWestEast() const;

    virtual void fill(grib_info&) const;
    virtual void fill(util::MeshGeneratorParameters&) const;
    virtual void reorder(long scanningMode, MIRValuesVector&) const;
    virtual void validate(const MIRValuesVector&) const;
    virtual void makeName(std::ostream&) const;
    virtual void print(std::ostream&) const;
    virtual bool extendBoundingBoxOnIntersect() const;

    virtual ::atlas::Grid atlasGrid() const;
    virtual Iterator* iterator() const;
    virtual size_t numberOfPoints() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

private:
    // -- Members

    long shapeOfTheEarth_;
    double radius_;
    double earthMajorAxis_;
    double earthMinorAxis_;
    bool shapeOfTheEarthProvided_;
};


}  // namespace regular
}  // namespace repres
}  // namespace mir


#endif
