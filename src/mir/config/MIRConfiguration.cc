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
    const eckit::Configuration& config = LibMir::instance().configuration();

    const eckit::PathName path = config.has("parameter-configuration") ? config.getString("parameter-configuration") : "~mir/etc/mir/parameter.yaml";
    const std::string label = config.has("parameter-configuration-fill") ? config.getString("parameter-configuration-fill") : "class";
    instance_.configure(path, label);

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


void MIRConfiguration::configure(const eckit::PathName& path, const std::string& label) {

    // skip if this was already done
    if (path == path_ && label == label_) {
        return;
    }

    clear();
    if (path.exists()) {
        eckit::Log::debug<LibMir>() << "MIRConfiguration: loading configuration from '" << path << "', dereferrencing '" << label << "'" << std::endl;

        std::ifstream in(path);
        if (!in) {
            throw eckit::CantOpenFile(path);
        }

        // Create hierarchy (using non-overwriting filling keys)
        eckit::YAMLParser parser(in);
        const eckit::ValueMap j = parser.parse();
        fill(j);

        if (label.length()) {
            fill(pick(label));
            clear("parameter-configuration-fill");
        }

        // log into itself
        // NOTE: path_ and label_ members could be avoided! :-)
        set("parameter-configuration", path);
        set("parameter-configuration-fill", label);
        path_ = path;
        label_ = label;

    } else if (!path.asString().empty()) {
        throw eckit::CantOpenFile(path);
    }

    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *this << std::endl;


    // Use defaults (non-overwriting)
    defaults.copyValuesTo(*this, false);
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration["
        <<  "path=" << path_
        << ",label=" << label_
        << ",";
    InheritParametrisation::print(out);
    out << "]";
}


}  // namespace config
}  // namespace mir

