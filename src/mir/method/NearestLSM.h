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


#ifndef mir_method_NearestLSM_h
#define mir_method_NearestLSM_h

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {


class NearestLSM: public MethodWeighted {
public:

    NearestLSM(const param::MIRParametrisation&);

    virtual ~NearestLSM();

private:

    virtual const char *name() const;

    virtual void assemble(context::Context&, WeightMatrix&, const GridSpace& in, const GridSpace& out) const;

    /// Update interpolation weigths matrix to account for field masked values
    virtual void applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const;

    virtual lsm::LandSeaMasks getMasks(const atlas::Grid& in, const atlas::Grid& out) const;

    virtual void print(std::ostream&) const;

};


}  // namespace method
}  // namespace mir


#endif

