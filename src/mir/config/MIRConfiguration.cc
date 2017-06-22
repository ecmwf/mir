/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/YAMLParser.h"
#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


namespace  {
struct Defaults : param::SimpleParametrisation {
    Defaults() {
        // these options are (can be) overridden by the configuration file

        set("style", "mars");
        set("legendre-loader", "mapped-memory");
        set("interpolator-loader", "file-io");
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

        set("spectral-mapping", "linear");

        set("tolerance", 1e-10);
    }
};
}  // (anonymous namespace)


MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


MIRConfiguration::MIRConfiguration() {

    // Always start with internal defaults, not from file
    configure("");
}


const param::MIRParametrisation& MIRConfiguration::lookup(const param::MIRParametrisation& metadata) const {
    long id = 0;
    metadata.get("paramId", id);
    return metadata.get("paramId", id)? lookup(id, metadata)
                                      : defaults();
}


const param::MIRParametrisation& MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {
    return root_.pick(paramId, metadata);
}


const param::MIRParametrisation& MIRConfiguration::defaults() const {
    return root_;
}


void MIRConfiguration::configure(const eckit::PathName& path) {

    // configure from file, skip if this was already done
    if (!path.asString().empty() && path.asString() != configPath_) {
        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "'" << std::endl;
        std::ifstream in(path.asString().c_str());
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::YAMLParser parser(in);
        const eckit::ValueMap j = parser.parse();
        root_.fill(j);

        std::string configuration_fill;
        if (root_.get("configuration-fill", configuration_fill) && configuration_fill.length()) {
            root_.fill(root_.pick(configuration_fill));
            root_.clear("configuration-fill");
        }

        std::string configuration_clear;
        if (root_.get("configuration-clear", configuration_clear) && configuration_clear.length()) {
            root_.clear(configuration_clear);
        }
        root_.clear("configuration-clear");
    }


    configPath_ = path;
    //    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << root_ << std::endl;

    // Use defaults (non-overwriting)
    Defaults().copyValuesTo(root_, false);
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration["
        <<  "configPath=" << configPath_
        << ",root=" << root_
        << "]";
}


}  // namespace config
}  // namespace mir

