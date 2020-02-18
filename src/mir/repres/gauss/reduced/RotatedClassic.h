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


#ifndef mir_repres_gauss_reduced_RotatedClassic_h
#define mir_repres_gauss_reduced_RotatedClassic_h

#include "mir/repres/gauss/reduced/Classic.h"
#include "mir/util/Rotation.h"


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {


class RotatedClassic : public Classic {
public:
    // -- Exceptions
    // None

    // -- Constructors

    RotatedClassic(size_t, const util::Rotation&, const util::BoundingBox& = util::BoundingBox(),
                   double angularPrecision = 0);

    // -- Destructor

    virtual ~RotatedClassic();  // Change to virtual if base class

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

    util::Rotation rotation_;

    // -- Methods

    void print(std::ostream&) const;  // Change to virtual if base class

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
    virtual void fill(grib_info&) const;
    virtual void fill(api::MIRJob&) const;
    virtual atlas::Grid atlasGrid() const;
    virtual const Gridded* croppedRepresentation(const util::BoundingBox&) const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const RotatedClassic& p)
    //  { p.print(s); return s; }
};


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif
