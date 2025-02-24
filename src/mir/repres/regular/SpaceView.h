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

#include <memory>

#include "mir/repres/regular/RegularGrid.h"


namespace mir::repres::regular {
namespace detail {


/*
 * References:
 * - [1] LRIT/HRIT Global Specification (CGMS 03, Issue 2.6, 12.08.1999)
 * - [2] MSG Ground Segment LRIT/HRIT Mission Specific Implementation, EUMETSAT Document, (EUM/MSG/SPE/057, Issue 6, 21.
 * June 2006)
 * - [3] MSG Ground Segment LRIT/HRIT Mission Specific Implementation, EUMETSAT Document, (EUM/MSG/SPE/057 v7 e-signed.
 * 30 November 2015)
 */


}  // namespace detail


class SpaceView final : public RegularGrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit SpaceView(const param::MIRParametrisation& param) : SpaceView{SpaceViewInternal(param)} {}

    // -- Destructor
    // None

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

private:
    // -- Types

    struct SpaceViewInternal {
        explicit SpaceViewInternal(const param::MIRParametrisation&);

        std::unique_ptr<Projection> projection;
        std::unique_ptr<Projection> projectionGreenwich;
        util::BoundingBox bbox;
        util::Shape shape;

        RegularGrid::LinearSpacing x;
        RegularGrid::LinearSpacing y;

        double LongestElementDiagonal;
        double Lop;
    };

    // -- Constructors

    SpaceView(SpaceViewInternal&&);

    // -- Members

    const std::unique_ptr<Projection> projectionGreenwich_;
    const double LongestElementDiagonal;
    const double Lop;
    mutable std::vector<PointLonLat> lonlat_;

    // -- Methods

    const std::vector<PointLonLat>& lonlat() const;

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    Iterator* iterator() const override;
    bool getLongestElementDiagonal(double&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::regular
