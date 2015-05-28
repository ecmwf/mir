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
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_Bilinear_H
#define mir_method_Bilinear_H

#include "mir/method/Nearest.h"


namespace eckit {
namespace geometry {
class Point3;
}
}


namespace mir {
namespace method {


class NearestLSM: public Nearest {
  public:

    NearestLSM(const param::MIRParametrisation &);

    virtual ~NearestLSM();

  protected:

    virtual void hash( eckit::MD5 & ) const;

  private:

    // -- Methods
    // None

    // -- Overridden methods

    const char *name() const;
    lsm::LandSeaMasks getMasks(const atlas::Grid &in, const atlas::Grid &out) const;
    size_t nclosest() const;
    void applyMasks(WeightMatrix &W, const lsm::LandSeaMasks &masks) const;
    void assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const;
    void print(std::ostream &) const;


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

};


}  // namespace method
}  // namespace mir

#endif

