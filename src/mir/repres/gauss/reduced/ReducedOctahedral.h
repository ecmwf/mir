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


#ifndef mir_repres_gauss_reduced_ReducedOctahedral_h
#define mir_repres_gauss_reduced_ReducedOctahedral_h

#include "mir/repres/gauss/reduced/Octahedral.h"


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


class ReducedOctahedral : public Octahedral {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ReducedOctahedral(size_t, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

    // -- Destructor

    virtual ~ReducedOctahedral();

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

    virtual Iterator* iterator() const;
    virtual const Gridded* croppedRepresentation(const util::BoundingBox&) const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif
