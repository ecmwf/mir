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
      // default "library" settings; they can be overridden by the main
      // configuration file or (some) user options
      set("paramId.u", 131);
      set("paramId.v", 132);

      set("parameter-configuration", "");
      set("parameter-configuration-fill", "");

      set("mir-cache-path", "mir-cache-path;$MIR_CACHE_PATH;/tmp/cache");

      // default "parameter-configuration" settings; they can be overridden by
      // the "parameter-configuration" file,  or (some) user options
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
} defaults;
}  // (anonymous namespace)


const std::string MIRConfiguration::defaultPath = "~mir/etc/mir/config.yaml";
std::string MIRConfiguration::path = defaultPath;


MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;

    // Make sure instance is configured
    instance_.configure(path);

    return instance_;
}


MIRConfiguration::MIRConfiguration() {
}


const param::MIRParametrisation& MIRConfiguration::lookup(const param::MIRParametrisation& metadata) const {
    long id = 0;
    metadata.get("paramId", id);
    return lookup(id, metadata);
}


const param::MIRParametrisation& MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {
    return pick(paramId, metadata);
}


void MIRConfiguration::configure(const eckit::PathName& path) {

    // skip if this was already done
    if (path == configuration_) {
        return;
    }


    // configure from file, ensuring a clean start
    clear();
    if (!path.asString().empty() && path != configuration_) {
        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "'" << std::endl;

        std::ifstream in(path);
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::YAMLParser parser(in);
        const eckit::ValueMap j = parser.parse();
        fill(j);
    }
    configuration_ = path;


    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *this << std::endl;


    // Fill parameter configuration
    std::string parameterConfiguration = "";
    get("parameter-configuration", parameterConfiguration);

    if (parameterConfiguration.length()) {
        const eckit::PathName path(parameterConfiguration);
        set("parameter-configuration", path);

        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading parameter configuration from '" << path << "'" << std::endl;

        std::ifstream in(path.asString().c_str());
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::YAMLParser parser(in);
        const eckit::ValueMap j = parser.parse();
        fill(j);

        std::string parameterFill;
        if (get("parameter-configuration-fill", parameterFill) && parameterFill.length()) {
            fill(pick(parameterFill));
            clear("parameter-configuration-fill");
        }
    }


    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *this << std::endl;


    // Use defaults (non-overwriting)
    defaults.copyValuesTo(*this, false);
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration["
        <<  "configuration=" << configuration_
        << ",";
    InheritParametrisation::print(out);
    out << "]";
}


}  // namespace config
}  // namespace mir

