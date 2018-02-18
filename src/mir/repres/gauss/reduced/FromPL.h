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


#ifndef FromPL_H
#define FromPL_H

#include "mir/repres/gauss/reduced/Reduced.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {
namespace gauss {
namespace reduced {

class FromPL : public Reduced {
public:

    // -- Exceptions
    // None

    // -- Contructors

    FromPL(const param::MIRParametrisation &);

    // -- Destructor

    virtual ~FromPL(); // Change to virtual if base class

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

    FromPL(size_t, const std::vector<long> &, const util::BoundingBox &);
    FromPL(const std::vector<long> &);

    // -- Members

    std::vector<long> pl_;

    // -- Methods
    // None

    // -- Overridden methods
    virtual void fill(grib_info &) const;
    virtual void fill(api::MIRJob &) const;
    virtual atlas::Grid atlasGrid() const;
    virtual const std::vector<long>& pls() const;
    virtual bool sameAs(const Representation& other) const;
    virtual void makeName(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    FromPL(const FromPL &);
    FromPL &operator=(const FromPL &);

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

    //friend ostream& operator<<(ostream& s,const FromPL& p)
    //  { p.print(s); return s; }

};


}  // namespace reduced
}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif

