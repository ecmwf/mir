/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#ifndef mir_method_StructuredLinear3D_h
#define mir_method_StructuredLinear3D_h

#include "mir/method/StructuredMethod.h"


namespace mir {
namespace method {


class StructuredLinear3D : public StructuredMethod {
public:

    StructuredLinear3D(const param::MIRParametrisation&);
    ~StructuredLinear3D();

private:

    void assembleStructuredInput(WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;

    const char* name() const;

    void hash(eckit::MD5&) const;

    void print(std::ostream&) const;

};


}  // namespace method
}  // namespace mir


#endif

