/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include "mir/method/structured/StructuredMethod.h"


namespace mir::method::structured {


struct StructuredBilinearLatLon final : StructuredMethod {
    explicit StructuredBilinearLatLon(const param::MIRParametrisation&);

private:
    void assembleStructuredInput(WeightMatrix&, const repres::Representation& in,
                                 const repres::Representation& out) const override;

    const char* name() const override;
    void print(std::ostream&) const override;
    bool sameAs(const Method&) const override;
};


}  // namespace mir::method::structured
