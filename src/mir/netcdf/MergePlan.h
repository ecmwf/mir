// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Step.h"

#include <map>
#include <queue>


namespace mir::netcdf {
class Variable;
class Dataset;
}  // namespace mir::netcdf


namespace mir::netcdf {


class MergePlan {
public:
    MergePlan(Dataset&);
    ~MergePlan();

    void add(Step*);
    void execute();

    void link(const Variable&, const Variable&);
    const Variable& link(const Variable&);

    Dataset& field() const;

private:
    Dataset& field_;

    MergePlan(const MergePlan&);
    MergePlan& operator=(const MergePlan&);

    // ----

    std::priority_queue<Step*, std::deque<Step*>, CompareSteps> queue_;
    std::vector<Step*> steps_;
    std::map<const Variable*, const Variable*> link_;
};


}  // namespace mir::netcdf
