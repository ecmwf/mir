// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "Job.h"

#include <string>


namespace mir_bridge {


void Job::set_from_string(rust::Str args) {
    set(std::string(args));
}


rust::String Job::to_json() const {
    return rust::String(json_str());
}


void Job::execute_one(MIRInput& input, MIROutput& output) const {
    execute(input.inner(), output.inner());
}


size_t Job::execute_all(MIRInput& input, MIROutput& output) const {
    size_t processed = 0;
    while (input.next()) {
        execute(input.inner(), output.inner());
        ++processed;
    }
    return processed;
}


std::unique_ptr<Job> Job::make() {
    return std::make_unique<Job>();
}


}  // namespace mir_bridge
