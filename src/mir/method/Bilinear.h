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


#ifndef mir_method_Bilinear_h
#define mir_method_Bilinear_h

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {


class Bilinear : public MethodWeighted {
public:

    Bilinear(const param::MIRParametrisation&);

    ~Bilinear();

private: // methods

    void assemble(WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;

    void print(std::ostream&) const;

    const char* name() const;

    void hash(eckit::MD5&) const;

};


}  // namespace method
}  // namespace mir


#endif

