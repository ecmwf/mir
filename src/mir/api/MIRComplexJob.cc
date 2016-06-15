/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "mir/api/MIRComplexJob.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/api/MIRWatcher.h"

#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/ActionGraph.h"
#include "mir/input/MIRInput.h"
#include "mir/log/MIR.h"
#include "mir/data/MIRField.h"


namespace mir {
namespace api {


MIRComplexJob::MIRComplexJob():
    input_(0) {
}


MIRComplexJob::~MIRComplexJob() {
    clear();
}

void MIRComplexJob::clear() {
    for (std::vector<action::Job *>::const_iterator j = jobs_.begin(); j != jobs_.end(); ++j) {
        delete (*j);
    }
    jobs_.clear();
    for (std::vector<api::MIRJob *>::const_iterator j = apis_.begin(); j != apis_.end(); ++j) {
        delete (*j);
    }
    apis_.clear();
    for (std::vector<api::MIRWatcher *>::const_iterator j = watchers_.begin(); j != watchers_.end(); ++j) {
        delete (*j);
    }
    watchers_.clear();
    input_ = 0;
}

// static void fill(size_t n) {

// }

void MIRComplexJob::execute(util::MIRStatistics& statistics) const {


    action::ActionGraph graph;

    size_t i = 0;
    for (std::vector<action::Job *>::const_iterator j = jobs_.begin(); j != jobs_.end(); ++j, ++i) {
        graph.add((*j)->plan(), watchers_[i]);
    }

    if (!input_) {
        return;
    }

    eckit::ScopedPtr< data::MIRField > field(input_->field());

    std::cout << ">>>>>>>>>>>> ====== " << std::endl;

    std::cout << *input_ << std::endl;
    // std::cout << *field << std::endl;

    graph.dump(std::cout, 1);
    graph.execute(*field, statistics);
    std::cout << "<<<<<<<<<<< ======" << std::endl;


}

bool MIRComplexJob::empty() const {
    return jobs_.empty();
}

void MIRComplexJob::print(std::ostream &out) const {
    out << "MIRComplexJob[]";
}

MIRComplexJob &MIRComplexJob::add(api::MIRJob *job,
                                  input::MIRInput &input,
                                  output::MIROutput &output,
                                  api::MIRWatcher *watcher) {

    if (!job) {
        return *this;
    }

    if (!input_) {
        input_ = &input;
    }

    if (input_ != &input) {
        std::ostringstream oss;
        oss << "MIRComplexJob: all jobs must share the same input (for now)";
        throw eckit::SeriousBug(oss.str());
    }


    apis_.push_back(job); // We keep it becase the Job needs a reference
    jobs_.push_back(new action::Job(*job, input, output));
    watchers_.push_back(watcher);

    return *this;
}


}  // namespace api
}  // namespace mir

