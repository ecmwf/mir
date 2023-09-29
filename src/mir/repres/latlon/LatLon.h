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

#include "mir/repres/Gridded.h"
#include "mir/util/Increments.h"


namespace mir::repres::latlon {


class LatLon : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    LatLon(const param::MIRParametrisation&);
    LatLon(const util::Increments&, const util::BoundingBox& = {}, const PointLatLon& reference = {0, 0});

    // -- Destructor

    ~LatLon() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t Ni() const { return ni_; }

    size_t Nj() const { return nj_; }

    static void globaliseBoundingBox(util::BoundingBox&, const util::Increments&,
                                     const PointLatLon& reference = {0, 0});

    static void correctBoundingBox(util::BoundingBox&, size_t& ni, size_t& nj, const util::Increments&,
                                   const PointLatLon& reference = {0, 0});

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    const util::Increments increments_;
    const PointLatLon reference_;
    size_t ni_;
    size_t nj_;

    // -- Methods
    // None

    // -- Overridden methods

    void fillGrib(grib_info&) const override;

    void fillJob(api::MIRJob&) const override;

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    void makeName(std::ostream&) const override;

    bool sameAs(const Representation&) const override;

    bool isPeriodicWestEast() const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    size_t numberOfPoints() const override;
    bool getLongestElementDiagonal(double&) const override;

    Representation* globalise(data::MIRField&) const override;

    // -- Class members

    class LatLonIterator {
        size_t ni_;
        size_t nj_;
        eckit::Fraction north_;
        eckit::Fraction west_;
        eckit::Fraction we_;
        eckit::Fraction ns_;
        size_t i_;
        size_t j_;
        Latitude latValue_;
        Longitude lonValue_;
        eckit::Fraction lat_;
        eckit::Fraction lon_;

    protected:
        size_t count_;
        bool first_;

        ~LatLonIterator();
        void print(std::ostream&) const;
        bool next(Latitude&, Longitude&);

    public:
        LatLonIterator(size_t ni, size_t nj, Latitude north, Longitude west, const util::Increments& increments);
    };

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    size_t frame(MIRValuesVector& values, size_t size, double missingValue, bool estimate = false) const override;

    void reorder(long scanningMode, MIRValuesVector& values) const override;

    void validate(const MIRValuesVector&) const override;

    const LatLon* croppedRepresentation(const util::BoundingBox&) const override;

    bool extendBoundingBoxOnIntersect() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::latlon
