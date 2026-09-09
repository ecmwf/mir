// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Step.h"


namespace mir::netcdf {
class Variable;
}  // namespace mir::netcdf


namespace mir::netcdf {


class MergeCoordinateStep : public Step {
public:
    MergeCoordinateStep(Variable& out, const Variable& in);

private:
    // Members

    Variable& out_;
    const Variable& in_;

    // -- Methods
    void print(std::ostream&) const override;
    int rank() const override;
    void execute(MergePlan&) override;
};


}  // namespace mir::netcdf
