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


#ifndef mir_repres_gauss_Gaussian_h
#define mir_repres_gauss_Gaussian_h

#include "mir/repres/Gridded.h"
#include "mir/repres/gauss/GaussianIterator.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {


class Gaussian : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Gaussian(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);
    Gaussian(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gaussian();  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    ///@return global Gaussian latitudes
    static const std::vector<double>& latitudes(size_t N);

    ///@return global Gaussian quadrature weights
    static const std::vector<double>& weights(size_t N);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    size_t N_;
    double angularPrecision_;

    // -- Methods

    ///@return global Gaussian latitudes
    const std::vector<double>& latitudes() const;

    ///@return global Gaussian quadrature weigths
    const std::vector<double>& weights() const;

    bool angleApproximatelyEqual(const Latitude&, const Latitude&) const;
    bool angleApproximatelyEqual(const Longitude&, const Longitude&) const;

    void correctSouthNorth(Latitude& s, Latitude& n, bool in = true) const;

    Iterator* unrotatedIterator(gauss::GaussianIterator::ni_type) const;
    Iterator* rotatedIterator(gauss::GaussianIterator::ni_type, const util::Rotation&) const;

    std::vector<double> calculateUnrotatedGridBoxLatitudeEdges() const;

    // -- Overridden methods

    virtual void fill(util::MeshGeneratorParameters&) const;
    virtual bool sameAs(const Representation&) const;
    virtual void validate(const MIRValuesVector&) const;
    virtual bool extendBoundingBoxOnIntersect() const;

    bool includesNorthPole() const;
    bool includesSouthPole() const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const Gaussian& p)
    //  { p.print(s); return s; }
};


}  // namespace repres
}  // namespace mir


#endif
