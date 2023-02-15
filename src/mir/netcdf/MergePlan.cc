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


#include "mir/netcdf/MergePlan.h"

#include <ostream>

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Variable.h"
#include "mir/util/Log.h"


namespace mir::netcdf {


MergePlan::MergePlan(Dataset& field) : field_(field) {}


MergePlan::~MergePlan() {
    for (auto& j : steps_) {
        delete j;
    }
}


void MergePlan::add(Step* s) {
    for (auto& j : steps_) {
        if (j->merge(s)) {
            delete s;
            return;
        }
    }
    queue_.push(s);
    steps_.push_back(s);
}


void MergePlan::execute() {
    while (!queue_.empty()) {
        Step* s = queue_.top();
        queue_.pop();
        s->execute(*this);
    }
}


void MergePlan::link(const Variable& out, const Variable& in) {
    ASSERT(link_.find(&out) == link_.end());
    link_[&out] = &in;
}


const Variable& MergePlan::link(const Variable& out) {
    if (link_.find(&out) == link_.end()) {
        Log::info() << "MergePlan::link cannot find: " << out << std::endl;
    }
    ASSERT(link_.find(&out) != link_.end());
    return *link_[&out];
}


Dataset& MergePlan::field() const {
    return field_;
}


}  // namespace mir::netcdf
