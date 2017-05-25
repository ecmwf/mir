/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_method_Bilinear_H
#define mir_method_Bilinear_H

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class Bilinear : public MethodWeighted {
public:

    Bilinear(const param::MIRParametrisation&);

    virtual ~Bilinear();

private: // methods

    virtual void execute(context::Context&, const atlas::Grid& in, const atlas::Grid& out) const;

    virtual void assemble(context::Context&, WeightMatrix&, const GridSpace& in, const GridSpace& out) const;

    virtual void print(std::ostream&) const;

    virtual const char* name() const;

    virtual void hash(eckit::MD5&) const;

private:  // variables

    bool precipitation_;

    bool precipitationNeighbour_;

    double precipitationThreshold_;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir


#endif

