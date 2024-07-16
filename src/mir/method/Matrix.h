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


namespace mir::method {


class Matrix final : public MethodWeighted {
public:
    // -- Constructors

    explicit Matrix(const param::MIRParametrisation&);

private:
    // -- Members

    std::string diskKey_;
    std::string memoryKey_;

    // -- Overridden methods

    // From MethodWeighted
    void json(eckit::JSON&) const override;
    const char* name() const override;

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;

    CacheKeys getDiskAndMemoryCacheKeys(const repres::Representation& in, const repres::Representation& out,
                                        const lsm::LandSeaMasks&) const override;

    int version() const override { return 0; }

    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;

    // -- Friends

    friend eckit::JSON& operator<<(eckit::JSON& s, const Matrix& o) {
        o.json(s);
        return s;
    }
};


}  // namespace mir::method
