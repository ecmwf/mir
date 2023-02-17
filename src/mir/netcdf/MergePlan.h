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
