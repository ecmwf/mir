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


#ifndef mir_method_PseudoLaplace_H
#define mir_method_PseudoLaplace_H

#include "mir/method/MethodWeighted.h"

namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class PseudoLaplace: public MethodWeighted {

    size_t nclosest_;  ///< Number of closest points to search for

public:

    PseudoLaplace(const param::MIRParametrisation&);

    virtual ~PseudoLaplace();

protected:

    virtual void hash( eckit::MD5& ) const;

private:

    virtual void assemble(context::Context& ctx, WeightMatrix &W, const GridSpace& in, const GridSpace& out) const;
    virtual void print(std::ostream&) const;
    virtual const char* name() const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

