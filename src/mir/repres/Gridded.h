/*
 * (C) Copyright 1996-2016 ECMWF.
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


#ifndef mir_repres_Gridded_h
#define mir_repres_Gridded_h

#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {


class Gridded : public Representation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gridded(const param::MIRParametrisation&);
    Gridded(const util::BoundingBox&);

    // -- Destructor

    virtual ~Gridded();

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

    // -- Contructors

    Gridded();

    // -- Members

    util::BoundingBox bbox_;
//    util::BoundingBox domain_;

    // -- Methods
    // None

    // -- Overridden methods

    util::Domain domain() const;

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

    virtual void setComplexPacking(grib_info&) const;

    virtual void setSimplePacking(grib_info&) const;

    virtual void setGivenPacking(grib_info&) const;

    virtual void cropToDomain(const param::MIRParametrisation &parametrisation, context::Context & ctx) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Gridded& p)
    //  { p.print(s); return s; }

};


}  // namespace repres
}  // namespace mir


#endif

