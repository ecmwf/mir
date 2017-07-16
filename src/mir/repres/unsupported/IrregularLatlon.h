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


#ifndef IrregularLatlon_H
#define IrregularLatlon_H

#include "mir/repres/Gridded.h"


namespace mir {
namespace repres {


class IrregularLatlon : public Gridded {
public:

// -- Exceptions
    // None

// -- Contructors

    IrregularLatlon(const param::MIRParametrisation&);

// -- Destructor

    virtual ~IrregularLatlon(); // Change to virtual if base class

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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

private:

    IrregularLatlon();

// No copy allowed

    IrregularLatlon(const IrregularLatlon&);
    IrregularLatlon& operator=(const IrregularLatlon&);

// -- Members;double

    double south_;
    double north_;
    double south_north_;

    double west_;
    double east_;
    double west_east_;

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

// -- Methods
    // None


// -- Overridden methods

    virtual size_t numberOfPoints() const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation& other) const;
    virtual void fill(grib_info&) const;
    virtual atlas::Grid atlasGrid() const;
    virtual util::Domain domain() const;
    virtual Iterator* iterator() const;
    virtual bool isPeriodicWestEast() const;
    virtual bool includesNorthPole() const;
    virtual bool includesSouthPole() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const IrregularLatlon& p)
    //  { p.print(s); return s; }

};


}  // namespace repres
}  // namespace mir
#endif

