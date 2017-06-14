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


#ifndef Gaussian_H
#define Gaussian_H

#include "mir/repres/Gridded.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {


class Gaussian : public Gridded {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Gaussian(size_t N);
    Gaussian(size_t N, const util::BoundingBox &);
    Gaussian(const param::MIRParametrisation &);

    // -- Destructor

    virtual ~Gaussian(); // Change to virtual if base class

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

    static std::vector<double> latitudes(size_t N);

  protected:

    // -- Members

    size_t N_;

    // -- Methods

    const std::vector<double> &latitudes() const;
    virtual bool sameAs(const Representation& other) const;

    // -- Overridden methods

    bool includesNorthPole() const;
    bool includesSouthPole() const;

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Gaussian(const Gaussian &);
    Gaussian &operator=(const Gaussian &);

    // -- Members

    mutable std::vector<double> latitudes_;

    // -- Methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const Gaussian& p)
    //  { p.print(s); return s; }

};


}  // namespace repres
}  // namespace mir


#endif

