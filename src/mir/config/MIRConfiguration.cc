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
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSONParser.h"
#include "mir/config/LibMir.h"
#include "mir/param/InheritParametrisation.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


namespace  {


struct Defaults : param::SimpleParametrisation {
    Defaults() {
        // these options are (can be) overridden by the configuration file

        set("configuration-fill", "");  // very meta
        set("configuration-skip", "");

        set("style", "mars");
        set("legendre-loader", "mapped-memory");
        set("executor", "simple");

        set("interpolation", "linear"); // The word 'method' is used in grib
        set("decomposition", "none");
        set("stats", "scalar");
        set("caching", true);

        set("prune-epsilon", 1e-10);
        set("nclosest", 4L);

        set("lsm-selection", "auto");
        set("lsm-interpolation", "nearest-neighbour");
        set("lsm-weight-adjustment", 0.2);
        set("lsm-value-threshold", 0.5);

        set("autoresol", false);
    }
};


}  // (anonymous namespace)


MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


void MIRConfiguration::configure(const eckit::PathName& path) {

    // initialize hierarchy
    root_.reset(new param::InheritParametrisation());
    ASSERT(root_);


    // configure from file, skip if this was already done
    if (!path.asString().empty() && path.asString() != configPath_) {
        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "'" << std::endl;
        std::ifstream in(path.asString().c_str());
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        eckit::JSONParser parser(in);
        const eckit::ValueMap j = parser.parse();

        // create hierarchy (using non-overwriting filling keys)
        root_->fill(j);
        std::string configuration_fill;
        if (root_->get("configuration-fill", configuration_fill) && configuration_fill.length()) {
            root_->fill(root_->pick(configuration_fill));
            root_->clear("configuration-fill");
        }
    }


    // use defaults (non-overwriting)
    Defaults().copyValuesTo(*root_, false);


    std::string configuration_skip;
    if (root_->get("configuration-skip", configuration_skip) && configuration_skip.length()) {
        root_->clear(configuration_skip);
    }
    root_->clear("configuration-skip");


    configPath_ = path;
    //    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *root_ << std::endl;
}


MIRConfiguration::MIRConfiguration() {

    // Always start with internal defaults, not from file
    configure("");
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration["
        <<  "configPath=" << configPath_
        << ",root=" << *root_
        << "]";
}


const param::MIRParametrisation* MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {

    // inherit from most-specific paramId/metadata individual and its parents
    param::SimpleParametrisation* param = new param::SimpleParametrisation();
    ASSERT(root_);
    root_->pick(paramId, metadata).inherit(*param);
    return param;
}


const param::MIRParametrisation* MIRConfiguration::defaults() const {

    // inherit from top-level only (where defaults are held)
    param::SimpleParametrisation* param = new param::SimpleParametrisation();
    ASSERT(root_);
    root_->inherit(*param);
    return param;
}


}  // namespace config
}  // namespace mir
