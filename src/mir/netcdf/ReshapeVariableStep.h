// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cstddef>

#include "mir/netcdf/Step.h"


namespace mir::netcdf {
class Variable;
class Dimension;
}  // namespace mir::netcdf


namespace mir::netcdf {


class ReshapeVariableStep : public Step {
public:
    ReshapeVariableStep(Variable& out, const Dimension& dimension, size_t growth);
    ~ReshapeVariableStep() override;

private:
    // Members

    Variable& out_;
    const Dimension& dimension_;
    size_t growth_;
    ReshapeVariableStep* next_;

    // -- Methods
    void print(std::ostream&) const override;
    int rank() const override;
    void execute(MergePlan&) override;
    bool merge(Step*) override;
};


}  // namespace mir::netcdf
