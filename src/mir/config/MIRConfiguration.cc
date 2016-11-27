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
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/JSONParser.h"
#include "mir/config/InheritParametrisation.h"
#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


namespace  {


struct Defaults : param::SimpleParametrisation {
    Defaults() {
        // these options are (can be) overridden by the configuration file

        set("configuration-fill", "class");  // very meta

        set("style", "mars");
        set("legendre-loader", "mapped-memory");
        set("executor", "simple");

        set("interpolation", "linear"); // The word 'method' is used in grib
        set("decomposition", "none");
        set("stats", "Scalar");
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
    eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "'" << std::endl;
    if (configPath_ == path.asString()) {
        return;
    } else if (!configPath_.empty()) {
        eckit::Log::warning() << "MIRConfiguration: configuration has happened before!'" << std::endl;
    }


    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }

    eckit::JSONParser parser(in);
    const eckit::ValueMap j = parser.parse();


    // create hierarchy and fill (not overwriting) with defaults
    root_.reset(new InheritParametrisation());
    ASSERT(root_);

    root_->fill(j);
    Defaults().copyValuesTo(*root_, false);


    configPath_ = path;
    //    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *root_ << std::endl;
}


void MIRConfiguration::configure() {
    configure("~mir/etc/mir/configuration.json");
}


MIRConfiguration::MIRConfiguration() {
    // Always start with defaults
    root_.reset(new InheritParametrisation());
    ASSERT(root_);
    Defaults().copyValuesTo(*root_, false);
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration["
        <<  "configPath=" << configPath_
        << ",root=" << *root_
        << "]";
}


const param::MIRParametrisation* MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {
    param::SimpleParametrisation* param = new param::SimpleParametrisation();


    // inherit from most-specific paramId/metadata individual and its parents
    ASSERT(root_);
    root_->pick(paramId, metadata).inherit(*param);


    // inherit from configurable "filling" key(s), ensuring we has a "filling" map
    std::string fill_root_label;
    if (param->get("configuration-fill", fill_root_label) && fill_root_label.length()) {
        param->clear("configuration-fill");

        const InheritParametrisation& fill_root = root_->pick(fill_root_label);
        if (!fill_root.matches(fill_root_label)) {
            std::ostringstream msg;
            msg << "MIRConfiguration: could not find (root) fill key '" << fill_root_label << "'";
            throw eckit::UserError(msg.str());
        }

        std::string fill_label;
        size_t check = 0;
        while (param->get(fill_root_label, fill_label)) {
            ASSERT(check++ < 50);
            param->clear(fill_root_label);

            const InheritParametrisation& fill = fill_root.pick(fill_label);
            if (!fill.matches(fill_label)) {
                std::ostringstream msg;
                msg << "MIRConfiguration: could not find fill key '" << fill_root_label << "'";
                throw eckit::UserError(msg.str());
            }
            fill.inherit(*param);
        }

    }

    return param;
}


const param::MIRParametrisation* MIRConfiguration::defaults() const {
    static param::SimpleParametrisation empty;
    return lookup(0, empty);
}


}  // namespace config
}  // namespace mir
