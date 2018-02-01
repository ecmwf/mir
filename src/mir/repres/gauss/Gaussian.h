/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef Gaussian_H
#define Gaussian_H

#include "mir/repres/Gridded.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {


class Gaussian : public Gridded {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gaussian(size_t N);
    Gaussian(size_t N, const util::BoundingBox &);
    Gaussian(const param::MIRParametrisation &);

    // -- Destructor

    virtual ~Gaussian(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    ///@return global Gaussian latitudes
    static const std::vector<double>& latitudes(size_t N);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    size_t N_;

    // -- Methods

    ///@return global Gaussian latitudes
    const std::vector<double>& latitudes() const;

    virtual bool sameAs(const Representation& other) const;
    virtual void adjustBoundingBoxEastWest(util::BoundingBox&) const = 0;
    virtual void adjustBoundingBoxNorthSouth(util::BoundingBox&) const;

    // -- Overridden methods

    void adjustBoundingBox(util::BoundingBox&) const;
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

    virtual std::string atlasMeshGenerator() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Gaussian& p)
    //  { p.print(s); return s; }

};


}  // namespace repres
}  // namespace mir


#endif

