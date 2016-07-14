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


#ifndef ReducedLL_H
#define ReducedLL_H

#include "mir/repres/Gridded.h"
#include "mir/util/BoundingBox.h"


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

    virtual ~ReducedLL(); // Change to virtual if base class

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

    virtual atlas::grid::Grid* atlasGrid() const;

    virtual atlas::grid::Domain atlasDomain() const;

    void print(std::ostream&) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    ReducedLL(const ReducedLL&);
    ReducedLL& operator=(const ReducedLL&);

    // -- Members

    util::BoundingBox bbox_;
    std::vector<long> pl_;
    size_t Nj_;

    // -- Methods

    virtual void cropToDomain(const param::MIRParametrisation &parametrisation, context::Context & ctx) const;

    // -- Overridden methods

    virtual void fill(grib_info&) const;

    virtual void fill(api::MIRJob &) const;

    virtual void validate(const std::vector<double>&) const;

    virtual Iterator* rotatedIterator() const; // After rotation

    virtual Iterator* unrotatedIterator() const; // Before rotation

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

