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
static struct Defaults : param::SimpleParametrisation {
    Defaults() {
      // default settings, overridden by file or (some) user options
      set("style", "mars");
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
} defaults;
}  // (anonymous namespace)


const std::string MIRConfiguration::defaultPath = "~mir/etc/mir/parameter.yaml";
std::string MIRConfiguration::path = defaultPath;


MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;

    // Make sure instance is configured
    const eckit::Configuration& config = LibMir::instance().configuration();
    const eckit::PathName path = config.has("parameter-configuration") ? config.getString("parameter-configuration") : defaultPath;
    instance_.configure(path);

    return instance_;
}


MIRConfiguration::MIRConfiguration() {
}


void MIRConfiguration::configure(const eckit::PathName& path) {

    // skip if this was already done
    std::string oldPath;
    if (get("parameter-configuration", oldPath) && path == oldPath) {
        return;
    }

    clear();
    if (path.exists()) {
        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "'" << std::endl;

        std::ifstream in(path);
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::YAMLParser parser(in);
        const eckit::ValueMap j = parser.parse();
        fill(j);

        // Dereference
        std::string label;
        if (get("parameter-configuration-fill", label) && label.length()) {
            fill(pick(label));
            clear("parameter-configuration-fill");
        }

        // log into itself
        set("parameter-configuration", path);

    } else if (!path.asString().empty()) {
        throw eckit::CantOpenFile(path);
    }

//    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *this << std::endl;


    // Use defaults (non-overwriting)
    defaults.copyValuesTo(*this, false);
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration[";
    InheritParametrisation::print(out);
    out << "]";
}


}  // namespace config
}  // namespace mir

