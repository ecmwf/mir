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


#ifndef mir_method_StructuredLinear_h
#define mir_method_StructuredLinear_h

#include "mir/method/StructuredMethod.h"


namespace mir {
namespace method {


class StructuredLinear : public StructuredMethod {
public:

    StructuredLinear(const param::MIRParametrisation&);
    ~StructuredLinear();

private:

    void assemble(WeightMatrix&, const atlas::grid::StructuredGrid& in, const util::MIRGrid& out) const;

    const char* name() const;

    void hash(eckit::MD5&) const;

    void print(std::ostream&) const;

};


}  // namespace method
}  // namespace mir


#endif

