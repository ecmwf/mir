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
#include "mir/action/Job.h"
#include "mir/output/MIROutput.h"
#include "mir/api/MIRJob.h"


namespace mir {
namespace api {


MIRComplexJob::MIRComplexJob() {
}


MIRComplexJob::~MIRComplexJob() {
    for (std::vector<action::Job *>::iterator j = jobs_.begin(); j != jobs_.end(); ++j) {
        delete (*j);
    }
    for (std::vector<api::MIRJob *>::iterator j = apis_.begin(); j != apis_.end(); ++j) {
        delete (*j);
    }
}


void MIRComplexJob::execute() const {

}


void MIRComplexJob::print(std::ostream &out) const {
    out << "MIRComplexJob[]";
}

MIRComplexJob &MIRComplexJob::add(api::MIRJob *job, input::MIRInput &input, output::MIROutput &output) {
    apis_.push_back(job);
    jobs_.push_back(new action::Job(*job, input, output));
    return *this;
}


}  // namespace api
}  // namespace mir

