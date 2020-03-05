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


#ifndef mir_repres_gauss_reduced_Octahedral_h
#define mir_repres_gauss_reduced_Octahedral_h

#include "mir/repres/gauss/reduced/Reduced.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


class Octahedral : public Reduced {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Octahedral(size_t, const util::BoundingBox& = util::BoundingBox(), double angularPrecision = 0);

    // -- Destructor

    virtual ~Octahedral();  // Change to virtual if base class

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
    // None

    // -- Overridden methods
    virtual void fill(grib_info&) const;
    virtual void fill(api::MIRJob&) const;
    virtual atlas::Grid atlasGrid() const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation& other) const;

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

    virtual void fill(util::MeshGeneratorParameters&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const Octahedral& p)
    //  { p.print(s); return s; }
};


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif
