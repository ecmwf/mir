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
    RegularGG(size_t N);
    RegularGG(size_t N, const util::BoundingBox&, bool correctBoundingBox);

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

    virtual const Gridded* cropped(const util::BoundingBox& bbox) const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation& other) const;
    virtual void initTrans(Trans_t& trans) const;
    virtual Iterator* iterator() const;

    //virtual void validate(const std::vector<double>&) const;

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

