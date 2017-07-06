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


#ifndef mir_action_interpolate_Gridded2LatLon_h
#define mir_action_interpolate_Gridded2LatLon_h

#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"
#include "mir/util/Shift.h"


namespace mir {
namespace action {


class Gridded2LatLon : public Gridded2GriddedInterpolation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Gridded2LatLon(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Gridded2LatLon();

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

    util::Increments increments_;
    util::Shift shift_;
    util::BoundingBox bbox_;


    // -- Methods
    // None

    // -- Overridden methods

    virtual bool sameAs(const Action&) const;


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

    //friend ostream& operator<<(ostream& s,const Gridded2LatLon& p)
    //  { p.print(s); return s; }

};


}  // namespace action
}  // namespace mir


#endif

