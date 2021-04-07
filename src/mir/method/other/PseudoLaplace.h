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

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {
namespace other {


class PseudoLaplace : public MethodWeighted {

    size_t nclosest_;  ///< Number of closest points to search for

public:
    PseudoLaplace(const param::MIRParametrisation&);

    ~PseudoLaplace() override;

protected:
    void hash(eckit::MD5&) const override;

private:
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    void print(std::ostream&) const override;
    const char* name() const override;
    bool sameAs(const Method&) const override;
};


}  // namespace other
}  // namespace method
}  // namespace mir
