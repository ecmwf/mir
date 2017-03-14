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


#ifndef LatLon_H
#define LatLon_H

#include "mir/repres/Gridded.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace repres {
namespace latlon {


class LatLon : public Gridded {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    LatLon(const param::MIRParametrisation&);
    LatLon(const util::BoundingBox&, const util::Increments&);

    // -- Destructor

    virtual ~LatLon();

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

    util::BoundingBox bbox_;
    util::Increments increments_;
    size_t ni_;
    size_t nj_;

    // -- Methods

    virtual Iterator* rotatedIterator() const; // After rotation

    virtual Iterator* unrotatedIterator() const; // Before rotation

    void setNiNj();

    // -- Overridden methods

    virtual atlas::grid::Domain domain() const;

    virtual atlas::grid::Domain domain(const util::BoundingBox&) const;

    virtual void fill(grib_info &) const;

    virtual void fill(api::MIRJob &) const;

    virtual void shape(size_t& ni, size_t& nj) const;

    virtual void print(std::ostream &) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    LatLon(const LatLon &);
    LatLon &operator=(const LatLon &);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

    virtual void reorder(long scanningMode, std::vector<double>& values) const;

    virtual void validate(const std::vector<double>&) const;

    virtual void cropToDomain(const param::MIRParametrisation &parametrisation, context::Context& ctx) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const LatLon& p)
    //  { p.print(s); return s; }

};


}  // namespace latlon
}  // namespace repres
}  // namespace mir


#endif

