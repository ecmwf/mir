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


#ifndef mir_repres_latlon_LatLon_h
#define mir_repres_latlon_LatLon_h

#include "mir/repres/Gridded.h"
#include "mir/util/Increments.h"


namespace mir {
namespace repres {
namespace latlon {


class LatLon : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    LatLon(const param::MIRParametrisation&);
    LatLon(const util::Increments&, const util::BoundingBox& = util::BoundingBox(),
           const PointLatLon& reference = PointLatLon(0, 0));

    // -- Destructor

    virtual ~LatLon() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t Ni() const { return ni_; }

    size_t Nj() const { return nj_; }

    static void globaliseBoundingBox(util::BoundingBox&, const util::Increments&,
                                     const PointLatLon& reference = PointLatLon(0, 0));

    static void correctBoundingBox(util::BoundingBox&, size_t& ni, size_t& nj, const util::Increments&,
                                   const PointLatLon& reference = PointLatLon(0, 0));

    static bool samePoints(const param::MIRParametrisation& user, const param::MIRParametrisation& field);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    util::Increments increments_;
    size_t ni_;
    size_t nj_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void fill(grib_info&) const;

    virtual void fill(api::MIRJob&) const;

    virtual void print(std::ostream&) const override;

    virtual void makeName(std::ostream&) const;

    virtual bool sameAs(const Representation& other) const;

    virtual bool isPeriodicWestEast() const;
    virtual bool includesNorthPole() const;
    virtual bool includesSouthPole() const;

    virtual size_t numberOfPoints() const;
    virtual bool getLongestElementDiagonal(double&) const;

    virtual Representation* globalise(data::MIRField&) const;

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
        size_t count_;
        Latitude latValue_;
        Longitude lonValue_;
        eckit::Fraction lat_;
        eckit::Fraction lon_;

    protected:
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

    virtual void fill(util::MeshGeneratorParameters&) const;

    virtual size_t frame(MIRValuesVector& values, size_t size, double missingValue, bool estimate = false) const;

    virtual void reorder(long scanningMode, MIRValuesVector& values) const;

    virtual void validate(const MIRValuesVector&) const;

    virtual const LatLon* croppedRepresentation(const util::BoundingBox&) const;

    virtual bool extendBoundingBoxOnIntersect() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const LatLon& p)
    //  { p.print(s); return s; }
};


}  // namespace latlon
}  // namespace repres
}  // namespace mir


#endif
