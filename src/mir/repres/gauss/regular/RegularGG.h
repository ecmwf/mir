/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef mir_repres_gauss_regular_RegularGG_h
#define mir_repres_gauss_regular_RegularGG_h

#include "mir/repres/gauss/regular/Regular.h"


namespace mir {
namespace repres {
namespace gauss {
namespace regular {


class RegularGG : public Regular {
public:

    // -- Exceptions
    // None

    // -- Constructors

    RegularGG(const param::MIRParametrisation&);
    RegularGG(size_t N, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

    // -- Destructor

    virtual ~RegularGG();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

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

    virtual const Gridded* croppedRepresentation(const util::BoundingBox&) const;
    virtual bool sameAs(const Representation&) const;
    virtual Iterator* iterator() const;
    virtual std::string factory() const;

    // From Representation
    std::vector<util::GridBox> gridBoxes() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const RegularGG& p)
    // { p.print(s); return s; }

};


}  // namespace regular
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif

