// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/MethodWeighted.h"

#include "eckit/filesystem/PathName.h"


namespace mir::method {


class Matrix final : public MethodWeighted {
public:
    explicit Matrix(const param::MIRParametrisation&);

private:
    const eckit::PathName path_;

    void json(eckit::JSON&) const override;
    const char* type() const override;

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;

    CacheKeys getDiskAndMemoryCacheKeys(const repres::Representation&, const repres::Representation&,
                                        const lsm::LandSeaMasks&) const override {
        return {path_, path_};
    }

    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;
};


}  // namespace mir::method
