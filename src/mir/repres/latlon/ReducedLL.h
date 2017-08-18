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


#ifndef mir_repres_latlon_ReducedLL_h
#define mir_repres_latlon_ReducedLL_h

#include "mir/repres/Gridded.h"


namespace mir {
namespace repres {
namespace latlon {


class ReducedLL : public Gridded {
public:

    // -- Exceptions
    // None

    // -- Contructors

    ReducedLL(const param::MIRParametrisation&);

    // -- Destructor

    ~ReducedLL(); // Change to virtual if base class

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

    atlas::Grid atlasGrid() const;
    std::string atlasMeshGenerator() const;

    bool isPeriodicWestEast() const;
    bool includesNorthPole() const;
    bool includesSouthPole() const;

    void print(std::ostream&) const; // Change to virtual if base class

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::vector<long> pl_;

    // -- Methods
    // None

    // -- Overridden methods

    void fill(grib_info&) const;

    void fill(api::MIRJob &) const;

    void validate(const std::vector<double>&) const;

    Iterator* iterator() const;

    void makeName(std::ostream&) const;
    bool sameAs(const Representation& other) const;

    virtual size_t numberOfPoints() const;
    virtual bool getLongestElementDiagonal(double&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const ReducedLL& p)
    // { p.print(s); return s; }

};


}  // namespace latlon
}  // namespace repres
}  // namespace mir


#endif

