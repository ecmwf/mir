/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_KNearest_H
#define mir_method_KNearest_H

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {


class Nearest: public MethodWeighted {

  public:

    Nearest(const param::MIRParametrisation&);

    virtual ~Nearest();

  protected:

    double epsilon_;

    virtual void hash( eckit::MD5& ) const;

  private:

// -- Methods

    virtual size_t nclosest() const = 0;

    virtual void assemble(WeightMatrix& W, const atlas::Grid& in, const atlas::Grid& out) const;
    // virtual void print(std::ostream&) const;
    virtual const char* name() const;

// -- Class members

// -- Friends


};

}  // namespace method
}  // namespace mir

#endif

