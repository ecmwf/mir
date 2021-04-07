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

#include "mir/netcdf/Step.h"


namespace mir {
namespace netcdf {
class Variable;
class Dimension;
}  // namespace netcdf
}  // namespace mir


namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir
