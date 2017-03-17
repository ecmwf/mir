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


#include "mir/config/AConfiguration.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSONParser.h"
#include "mir/config/LibMir.h"
#include "mir/param/InheritParametrisation.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


void AConfiguration::configure(const eckit::PathName& path) {


    // configure from file, skip if this was already done
    if (!path.asString().empty() && path.asString() != configPath_) {
        eckit::Log::debug<LibMir>() << "AConfiguration: loading configuration from '" << path << "'" << std::endl;
        std::ifstream in(path.asString().c_str());
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::JSONParser parser(in);
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
    //    eckit::Log::debug<LibMir>() << "AConfiguration: " << *root_ << std::endl;
}


AConfiguration::AConfiguration() {

    // Always start with internal defaults, not from file
    configure("");
}


void AConfiguration::print(std::ostream& out) const {
    out << "AConfiguration["
        <<  "configPath=" << configPath_
        << ",root=" << root_
        << "]";
}


}  // namespace config
}  // namespace mir
