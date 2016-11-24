/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#include "mir/config/MIRConfiguration.h"

#include <iostream>
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/JSONParser.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace config {


const MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


MIRConfiguration::MIRConfiguration() {

    configFile_ = "configuration.json";
    configDir_  = "~mir/etc/mir";


    // open and parse configuration file
    eckit::PathName path(configFile_);
    if (!path.exists()){
        path = eckit::PathName(configDir_) / path;
    }

    eckit::Log::debug<LibMir>() << "Loading configuration from '" << path << "'" << std::endl;
    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }

    eckit::JSONParser parser(in);
    const eckit::ValueMap j = parser.parse();


    // create hierarchy
    root_.reset(new InheritParametrisation());
    root_->fill(j);
    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *root_ << std::endl;


    eckit::Log::info() << "done" << std::endl;
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration[...]";
}


const param::MIRParametrisation* MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata, const std::string& fillKey) const {
    param::SimpleParametrisation* param = new param::SimpleParametrisation();


    // inherit from most-specific paramId/metadata individual and its parents
    ASSERT(root_);
    root_->pick(paramId, metadata).inherit(*param);


    // inherit from "filling" key(s), ensuring we has a "filling" map
    if (fillKey.length()) {
        const InheritParametrisation& fill = root_->pick(fillKey);
        ASSERT(fillKey == fill.label());

        std::string fillLabel;
        size_t check = 0;
        while (param->get(fillKey, fillLabel)) {
            ASSERT(check++ < 50);
            param->clear(fillKey);
            fill.pick(fillLabel).inherit(*param);
        }
    }

    return param;
}


const param::MIRParametrisation* MIRConfiguration::lookupDefaults(const std::string& fillKey) const {
    static param::SimpleParametrisation empty;
    return lookup(0, empty, fillKey);
}


}  // namespace config
}  // namespace mir
